/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor.h"
#include "stdio.h"
#include <math.h>
#include <string.h>
#include "encoder.h"
#include "tracking.h"

#include "pid.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


#include <stdio.h> // 引入标准库用于sprintf
#include <stdlib.h> // 引入标准库用于atoi
#include <string.h> // 引入标准库用于memset

#define RX_BUFFER_SIZE 32 // 定义最大接收长度，例如 "F10000#"

uint8_t aRxBuffer;            // 每次接收1个字节的临时变量
uint8_t RxBuffer[RX_BUFFER_SIZE]; // 存放完整指令的缓冲区
uint8_t RxIndex = 0;          // 缓冲区索引
uint8_t CmdReceived_Flag = 0; // 命令接收完成标志位

// 记录当前的频率和占空比（用于查询时回复）
uint32_t Current_Freq = 1000; // 默认1kHz
uint8_t Current_Duty = 50;    // 默认50%


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* 定义全局句柄，确保它是对应 TIM1 的 */
extern TIM_HandleTypeDef htim1;

/**
 * @brief  设置PWM频率和占空比
 * @param  freq_hz: 目标频率 (单位: Hz)，例如 5000 代表 5kHz
 * @param  duty:    目标占空比 (0-100)，例如 50 代表 50%
 * @note   基于 168MHz 主频计算，会将 PSC 强制设为 0 以保证分辨率
 */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void UART_SendString(char *str)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
}

// 2. 核心：修改PWM频率和占空比的函数（已修正）
void Set_PWM_Param(uint32_t freq, uint8_t duty)
{
    // 防止非法输入
    if (freq == 0) freq = 1;
    if (duty > 100) duty = 100;

    // ---------------------------------------------------------
    // 关键修正：为了防止溢出，我们强制将定时器分频到 1MHz 计数频率
    // PSC = 167 (即 168MHz / 168 = 1MHz)
    // ---------------------------------------------------------
    __HAL_TIM_SET_PRESCALER(&htim1, 167); 

    // 计算 ARR (自动重装载值)
    // 1MHz 时钟下，ARR = 1,000,000 / Freq - 1
    uint32_t arr_value = (1000000 / freq) - 1;

    // 计算 CCR (比较值)
    uint32_t ccr_value = (arr_value + 1) * duty / 100;

    // 写入寄存器
    __HAL_TIM_SET_AUTORELOAD(&htim1, arr_value);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr_value);

    // 更新全局变量记录
    Current_Freq = freq;
    Current_Duty = duty;
}

// 3. 串口中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        if (CmdReceived_Flag == 0) 
        {
            if (aRxBuffer == '#') 
            {
                RxBuffer[RxIndex] = '\0'; // 封口
                CmdReceived_Flag = 1;     // 通知主循环
            }
            else
            {
                RxBuffer[RxIndex++] = aRxBuffer;
                // 防溢出：如果满了还没收到#，就清空重来
                if (RxIndex >= RX_BUFFER_SIZE) 
                {
                    RxIndex = 0; 
                    memset(RxBuffer, 0, RX_BUFFER_SIZE); 
                }
            }
        }
        HAL_UART_Receive_IT(&huart3, &aRxBuffer, 1);
    }
}

// 4. 命令处理函数 

void Process_Command(void)
{
    char tx_buffer[64];
    char *pCmd = NULL;

    // --- 指令1：M (Motor) 设置全车速度 ---
    // 格式：M500# (前进), M-300# (后退)
    if ( (pCmd = strchr((char*)RxBuffer, 'M')) != NULL || (pCmd = strchr((char*)RxBuffer, 'm')) != NULL )
    {
        int16_t target_speed = atoi(pCmd + 1); // 读取后面的数字（支持负数）
        
        // 设置所有电机
        Motor_SetSpeed(MOTOR_LF, target_speed);
        Motor_SetSpeed(MOTOR_LR, target_speed);
        Motor_SetSpeed(MOTOR_RF, target_speed);
        Motor_SetSpeed(MOTOR_RR, target_speed);

        sprintf(tx_buffer, "CMD: All Motors Set to %d\r\n", target_speed);
    }
    // --- 指令2：S (Stop) 停车 ---
    // 格式：S#
    else if ( (strchr((char*)RxBuffer, 'S')) != NULL || (strchr((char*)RxBuffer, 's')) != NULL )
    {
        Motor_Stop();
        sprintf(tx_buffer, "CMD: Emergency Stop!\r\n");
    }
    // --- 指令3：? 查询状态 ---
    else if ( strchr((char*)RxBuffer, '?') != NULL )
    {
        // 这里的 Car_Speed_L/R 是在 encoder.c 里计算的全局变量
        sprintf(tx_buffer, "Status: L=%.1f cm/s, R=%.1f cm/s\r\n", Car_Speed_L, Car_Speed_R);
    }
    else
    {
        sprintf(tx_buffer, "Err: Unknown Cmd\r\n");
    }

    UART_SendString(tx_buffer);
    
    // 清空缓冲区
    memset(RxBuffer, 0, RX_BUFFER_SIZE); 
    RxIndex = 0;
}

/**
 * @brief 应用层：寻迹任务启动函数
 * @note  这个函数一旦调用，就不会返回（内部有死循环）
 */
void App_Trace_Start(void)
{
    // 1. 发送提示信息
    // UART_SendString("=== System Init OK ===\r\n");
    // UART_SendString("Trace Mode will start in 2 seconds...\r\n");

    // 2. 安全倒计时 (给手拿开的时间)
    // 如果你有LED，可以在这里闪烁 LED
    HAL_Delay(1000); 
    //UART_SendString("Ready... 1\r\n");
   // HAL_Delay(1000);
   // UART_SendString("GO!\r\n");

    // 3. (可选) 如果你想在这里临时修改参数，
    // 由于 tracking.c 用的是宏定义，不能动态改。
    // 如果以后想动态改，需要把 tracking.c 里的宏改成全局变量。
    // 目前我们直接使用 tracking.h 里的默认参数。

    // 4. 进入死循环执行循迹
    while (1)
    {
        // 调用底层的核心循迹算法
        Tracking_Handler();

        // 控制循环频率
        // 10ms 大约是 100Hz，对于循迹小车足够了
        // 如果太快，电机反应不过来；如果太慢，过弯会冲出跑道
        HAL_Delay(10); 
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  Motor_Init();
  Encoder_Init();

  PID_Init(&PID_L);
  PID_Init(&PID_R);

  // HAL_UART_Receive_IT(&huart3, &aRxBuffer, 1);
  // UART_SendString("=== Auto Start Mode ===\r\n");
  App_Trace_Start();
   //int16_t default_speed = 300; 

   //Motor_SetSpeed(MOTOR_LF, default_speed);
   //Motor_SetSpeed(MOTOR_LR, default_speed);
   //Motor_SetSpeed(MOTOR_RF, default_speed);
  // Motor_SetSpeed(MOTOR_RR, default_speed);


  // 4. 停车
 // Motor_Stop();
// HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
// Set_PWM_Param(500, 30);
// HAL_UART_Receive_IT(&huart3, &aRxBuffer, 1);
//HAL_UART_Transmit_IT(&huart3, (uint8_t *)"PWM Controller Ready\r\n", 22);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // if (CmdReceived_Flag) // 如果收到了完整指令
    // {
    //     Process_Command(); // 并在内部处理PWM
    //     CmdReceived_Flag = 0;
    // }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// 串口接收中断回调函数

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
