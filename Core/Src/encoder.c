#include "encoder.h"
#include "tim.h"
#include "motor.h"
#include "pid.h"   // 引入 PID
#include "usart.h" // 用于打印
#include <stdio.h>
#include <string.h>
#include <math.h>

volatile int32_t Encoder_Count_L = 0;
volatile int32_t Encoder_Count_R = 0;

float Car_Speed_L = 0.0f;
float Car_Speed_R = 0.0f;

// 全局目标速度
float Target_Speed_L = 0.0f; 
float Target_Speed_R = 0.0f; 

static uint16_t Encoder_Timer_Tick = 0;

void Encoder_Init(void)
{
    HAL_TIM_Base_Start_IT(&htim10);
    Encoder_Count_L = 0;
    Encoder_Count_R = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ENC_L_Pin) // PI5
    {
        if (Motor_Dir_L > 0) Encoder_Count_L++;
        else if (Motor_Dir_L < 0) Encoder_Count_L--;
    }
    if (GPIO_Pin == ENC_R_Pin) // PI6
    {
        if (Motor_Dir_R > 0) Encoder_Count_R++;
        else if (Motor_Dir_R < 0) Encoder_Count_R--;
    }
}

// --- 定时器中断 (50ms 一次) ---
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM10)
    {
        Encoder_Timer_Tick++;

        if (Encoder_Timer_Tick >= SPEED_CALC_PERIOD)
        {
            Encoder_Timer_Tick = 0;

            // 1. 计算速度 (cm/s)
            // 注意：因为周期变 50ms (0.05s)，所以乘以 20 (1秒/0.05秒)
            int32_t count_L = Encoder_Count_L;
            Encoder_Count_L = 0;
            Car_Speed_L = (float)(count_L * 20 * WHEEL_CIRCUMFERENCE) / PULSES_PER_REV;

            int32_t count_R = Encoder_Count_R;
            Encoder_Count_R = 0;
            Car_Speed_R = (float)(count_R * 20 * WHEEL_CIRCUMFERENCE) / PULSES_PER_REV;

            // 2. 速度闭环 PID 计算
            float pwm_L = PID_Compute(&PID_L, Target_Speed_L, Car_Speed_L);
            float pwm_R = PID_Compute(&PID_R, Target_Speed_R, Car_Speed_R);

            // 3. 执行电机控制
            Motor_SetSpeed(MOTOR_LF, (int16_t)pwm_L);
            Motor_SetSpeed(MOTOR_LR, (int16_t)pwm_L);
            Motor_SetSpeed(MOTOR_RF, (int16_t)pwm_R);
            Motor_SetSpeed(MOTOR_RR, (int16_t)pwm_R);

            // 4. 调试打印 (建议用整数打印以免 float 问题)
            // 格式: T=目标 | V=实际 | P=PWM
            /*
            char tx_buffer[64];
            sprintf(tx_buffer, "T:%d V:%d P:%d\r\n", 
                   (int)Target_Speed_L, (int)Car_Speed_L, (int)pwm_L);
            HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 10);
            */
        }
    }
}