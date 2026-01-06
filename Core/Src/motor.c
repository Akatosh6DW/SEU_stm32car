#include "motor.h"
#include "tim.h"
#include "gpio.h"

// 引入外部定义的定时器句柄
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

int8_t Motor_Dir_L = 0; // 全局变量定义
int8_t Motor_Dir_R = 0;
/**
 * @brief  初始化电机：开启PWM输出，拉高使能引脚
 */
void Motor_Init(void)
{
    // 1. 开启 TIM1 的 4 个通道 (左侧前后)
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    // 2. 开启 TIM3 的 2 个通道 (右后)
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    // 3. 开启 TIM4 的 2 个通道 (右前)
    // 注意：请根据 CubeMX 实际配置确认 PD12/PD13 是 CH1/CH2 还是 CH3/CH4
    // 这里按标准 F407 定义为 CH1/CH2
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

    // 4. 拉高 DRV8848 使能引脚 (解除休眠)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); // Enable Left
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); // Enable Right
}

/**
 * @brief  停止所有电机
 */
void Motor_Stop(void)
{
    Motor_SetSpeed(MOTOR_LF, 0);
    Motor_SetSpeed(MOTOR_LR, 0);
    Motor_SetSpeed(MOTOR_RF, 0);
    Motor_SetSpeed(MOTOR_RR, 0);
}

// /**
//  * @brief  设置单个电机的速度和方向
//  * @param  motor_id: 电机ID (MOTOR_LF, MOTOR_LR, ...)
//  * @param  speed: 速度值 (-1000 到 1000)
//  * 正数: 正转
//  * 负数: 反转
//  * 0:    停止
//  */

void Motor_SetSpeed(Motor_ID_t motor_id, int16_t speed)
{
    TIM_HandleTypeDef *htim;
    uint32_t ch_a, ch_b;
    uint32_t pwm_val;

    // 1. 限制 PWM 范围
    if (speed > MOTOR_MAX_PWM) speed = MOTOR_MAX_PWM;
    if (speed < -MOTOR_MAX_PWM) speed = -MOTOR_MAX_PWM;

    // --- 记录方向 (供编码器使用) ---
    int8_t temp_dir = 0;
    if (speed > 0) temp_dir = 1;
    else if (speed < 0) temp_dir = -1;
    else temp_dir = 0;

    // 2. 根据 ID 选择定时器和通道
    switch (motor_id)
    {
    case MOTOR_LF: // 左前 (Left Front) - 保持原样
        htim = &htim1;
        ch_a = TIM_CHANNEL_2;
        ch_b = TIM_CHANNEL_1;
        Motor_Dir_L = temp_dir; 
        break;

    case MOTOR_LR: // 左后 (Left Rear) - 【修改这里！】
        htim = &htim1;
        // 原来是 ch_a=CH3, ch_b=CH4
        // 现在交换它们，实现软件反向
        ch_a = TIM_CHANNEL_3; // <--- 交换
        ch_b = TIM_CHANNEL_4; // <--- 交换
        Motor_Dir_L = temp_dir; 
        break;

    case MOTOR_RF: // 右前 (Right Front) - 保持原样
        htim = &htim4;
        ch_a = TIM_CHANNEL_2; 
        ch_b = TIM_CHANNEL_1;
        Motor_Dir_R = temp_dir;
        break;

    case MOTOR_RR: // 右后 (Right Rear) - 【修改这里！】
        htim = &htim3;
        // 原来是 ch_a=CH3, ch_b=CH4
        // 现在交换它们，实现软件反向
        ch_a = TIM_CHANNEL_3; // <--- 交换
        ch_b = TIM_CHANNEL_4; // <--- 交换
        Motor_Dir_R = temp_dir; 
        break;
        
    default:
        return;
    }

    // 3. 设置 PWM 占空比 (逻辑不用变，上面已经交换过通道了)
    if (speed > 0)
    {
        pwm_val = (uint32_t)speed;
        __HAL_TIM_SET_COMPARE(htim, ch_a, pwm_val);
        __HAL_TIM_SET_COMPARE(htim, ch_b, 0);
    }
    else if (speed < 0)
    {
        pwm_val = (uint32_t)(-speed); 
        __HAL_TIM_SET_COMPARE(htim, ch_a, 0);
        __HAL_TIM_SET_COMPARE(htim, ch_b, pwm_val);
    }
    else // 停止
    {
        __HAL_TIM_SET_COMPARE(htim, ch_a, 0);
        __HAL_TIM_SET_COMPARE(htim, ch_b, 0);
    }
}
