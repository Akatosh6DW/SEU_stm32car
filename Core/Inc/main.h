/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TRACK0_Pin GPIO_PIN_0
#define TRACK0_GPIO_Port GPIOF
#define TRACK1_Pin GPIO_PIN_1
#define TRACK1_GPIO_Port GPIOF
#define TRACK2_Pin GPIO_PIN_2
#define TRACK2_GPIO_Port GPIOF
#define TRACK7_Pin GPIO_PIN_3
#define TRACK7_GPIO_Port GPIOF
#define TRACK6_Pin GPIO_PIN_4
#define TRACK6_GPIO_Port GPIOF
#define TRACK5_Pin GPIO_PIN_5
#define TRACK5_GPIO_Port GPIOF
#define TRACK4_Pin GPIO_PIN_6
#define TRACK4_GPIO_Port GPIOF
#define TRACK3_Pin GPIO_PIN_7
#define TRACK3_GPIO_Port GPIOF
#define DRIVER_EN_L_Pin GPIO_PIN_11
#define DRIVER_EN_L_GPIO_Port GPIOB
#define DRIVER_EN_R_Pin GPIO_PIN_14
#define DRIVER_EN_R_GPIO_Port GPIOD
#define ENC_L_Pin GPIO_PIN_5
#define ENC_L_GPIO_Port GPIOI
#define ENC_L_EXTI_IRQn EXTI9_5_IRQn
#define ENC_R_Pin GPIO_PIN_6
#define ENC_R_GPIO_Port GPIOI
#define ENC_R_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
