/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32l1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"
#include "Keypad.h"
#include "fonts.h"
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
#define F_Pin GPIO_PIN_14
#define F_GPIO_Port GPIOC
#define D_Pin GPIO_PIN_15
#define D_GPIO_Port GPIOC
#define E_Pin GPIO_PIN_0
#define E_GPIO_Port GPIOC
#define C_Pin GPIO_PIN_2
#define C_GPIO_Port GPIOC
#define A_Pin GPIO_PIN_3
#define A_GPIO_Port GPIOC
#define PER_EN_Pin GPIO_PIN_2
#define PER_EN_GPIO_Port GPIOA
#define ST7735_SCK_Pin GPIO_PIN_5
#define ST7735_SCK_GPIO_Port GPIOA
#define TRGOUT_Pin GPIO_PIN_6
#define TRGOUT_GPIO_Port GPIOA
#define ST7735_SDA_Pin GPIO_PIN_7
#define ST7735_SDA_GPIO_Port GPIOA
#define ST7735_RES_Pin GPIO_PIN_4
#define ST7735_RES_GPIO_Port GPIOC
#define ST7735_DC_Pin GPIO_PIN_5
#define ST7735_DC_GPIO_Port GPIOC
#define ST7735_CS_Pin GPIO_PIN_0
#define ST7735_CS_GPIO_Port GPIOB
#define ST7735_BL_Pin GPIO_PIN_1
#define ST7735_BL_GPIO_Port GPIOB
#define K_1_Pin GPIO_PIN_8
#define K_1_GPIO_Port GPIOC
#define K_2_Pin GPIO_PIN_9
#define K_2_GPIO_Port GPIOC
#define K_5_Pin GPIO_PIN_15
#define K_5_GPIO_Port GPIOA
#define K_4_Pin GPIO_PIN_10
#define K_4_GPIO_Port GPIOC
#define K_3_Pin GPIO_PIN_11
#define K_3_GPIO_Port GPIOC
#define B_Pin GPIO_PIN_12
#define B_GPIO_Port GPIOC
#define LED_Pin GPIO_PIN_8
#define LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
