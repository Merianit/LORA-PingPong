/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include <stdlib.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7735.h"
#include "st7735_test.h"
#include "config.h"
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#include "ctype.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct _Ball{
    uint8_t x;
    uint8_t y;
    int8_t x_dir;
    int8_t y_dir;
    uint8_t new_x;
    uint8_t new_y;
    uint8_t radius;
}Ball; 

typedef struct _Player{
    uint8_t x;
    uint8_t y;
    uint8_t score;
}Player;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// ��������� ��� ������ � �����������
Keypad_t keypad;
Ball ball;
Player player;
Player bot;

const uint8_t PADDLE_HEIGHT = 30; //Высота ракетки
const uint8_t MAX_SCORE = 8; // Победное число очков
bool GameIsRunning = true;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
int random_int(int low, int high); 
void draw_court();
void end_of_game(int);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  //Первичная инициализация структур
    ball.x = 0;
    ball.y = 0;
    ball.new_x = 0;
    ball.new_y = 0;
    ball.x_dir = 0;
    ball.y_dir = 0;
    ball.radius = 3;
    
    player.score = 0;
    player.x = 10;
    player.y = 64;
    
    bot.score = 0;
    bot.x = 150;
    bot.y = 64;
    
    HAL_GPIO_WritePin(PER_EN_GPIO_Port, PER_EN_Pin, GPIO_PIN_SET);

    ST7735_Init(Displ_Orientat_90); 
    ST7735_SetBacklight('F');
    ST7735_FillScreen(ST7735_BLACK);
    
    ball.x = random_int(0,160);
    ball.y = random_int(0,128);
    ball.x_dir = 1;
    ball.y_dir = 1;
    
    uint8_t half_paddle = PADDLE_HEIGHT >> 1;
    uint8_t cycles = 0;
    
    bool ResetBall = false;
    bool up = false;
    bool down = false;
    srand(HAL_GetTick());
    
    HAL_GPIO_WritePin(F_GPIO_Port, F_Pin, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    up = !HAL_GPIO_ReadPin(K_4_GPIO_Port, K_4_Pin);
    down =  !HAL_GPIO_ReadPin(K_5_GPIO_Port, K_5_Pin);
      
    if(ResetBall){
        
        ST7735_FillCircle(ball.x, ball.y, ball.radius, ST7735_BLACK);
        ball.x = 80;
        ball.y = 64;
        
        do{
            ball.x_dir = random_int(-1,2);
        }while(ball.x_dir == 0);
         do{
            ball.y_dir = random_int(-1,2);
        }while(ball.y_dir == 0);
         
        ResetBall = false;
    }
    
    if(GameIsRunning){
        cycles++;
        ST7735_DrawWString(10, 10, "Player1: ",  Font12, 1, ST7735_WHITE, ST7735_BLACK);
        ST7735_DrawWChar(70, 10, (char)player.score + '0', Font12, 1, ST7735_WHITE, ST7735_BLACK);
        ST7735_DrawWString(90, 10, "Bot: ",  Font12, 1, ST7735_WHITE, ST7735_BLACK);
        ST7735_DrawWChar(150, 10, (char)bot.score + '0', Font12, 1, ST7735_WHITE, ST7735_BLACK);
        
        ball.new_x = ball.x + ball.x_dir;
        ball.new_y = ball.y + ball.y_dir;
        //Проверка на столкновение с вертиальной стеной
        if(ball.new_x == 0 ){ 
            bot.score++;
            if(bot.score == 8)
                end_of_game(1);
            ResetBall = true;
        }
        else if(ball.new_x == 160)
        {
            player.score++;
            if(player.score == 8)
                end_of_game(2);
            ResetBall = true;
        }
    
        //Проверка на столкновение с горизонтальной стеной
        if(ball.new_y == 0 || ball.new_y == 120){
            ball.y_dir = -ball.y_dir; //Меняем направление
            ball.new_y += 2*ball.y_dir; //Откатываемся назад
        }
        //Проверка на столкновение с ракеткой игрока
        if(ball.new_x == player.x && ball.new_y >= player.y - half_paddle && ball.new_y <= player.y + half_paddle)
        {
            ball.x_dir = -ball.x_dir; //Меняем направление
            ball.new_x += 2*ball.x_dir; //Откатываемся назад
        }
        //Проверка на столкновение с ракеткой бота
        if(ball.new_x == bot.x && ball.new_y >= bot.y - half_paddle && ball.new_y <= bot.y + half_paddle)
        {
            ball.x_dir = -ball.x_dir; //Меняем направление
            ball.new_x += 2*ball.x_dir; //Откатываемся назад
        }
        ST7735_FillCircle(ball.x, ball.y, ball.radius, ST7735_BLACK);
        ST7735_FillCircle(ball.new_x, ball.new_y, ball.radius, ST7735_RED);
        ball.x = ball.new_x;
        ball.y = ball.new_y;
        draw_court();
    
        //Ракетка игрока
        ST7735_DrawLine(player.x, player.y - half_paddle, player.x, player.y + half_paddle, ST7735_BLACK);
        if(down)
        {
            player.y++;
            up = false;
        }
        else if(up)
        {
            player.y--;
            down = false;
        }
        if(player.y - half_paddle < 1) player.y = 1 + half_paddle;
        if(player.y +  half_paddle > 120) player.y = 120 - half_paddle;
        ST7735_DrawLine(player.x, player.y - half_paddle, player.x, player.y + half_paddle, ST7735_GREEN);
        //Ракетка бота
        if (cycles > 1){ // Замедляем бота
        ST7735_DrawLine(bot.x, bot.y - half_paddle, bot.x, bot.y + half_paddle, ST7735_BLACK);
        if(bot.y > ball.y){
            bot.y--;
        }
        if(bot.y < ball.y){
            bot.y++;
        }
        if(bot.y - half_paddle < 1) bot.y = 1 + half_paddle;
        if(bot.y +  half_paddle > 120) bot.y = 120 - half_paddle;
        ST7735_DrawLine(bot.x, bot.y - half_paddle, bot.x, bot.y + half_paddle, ST7735_YELLOW);
        cycles = 0;
    }
    }
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int random_int(int low, int high)
{
   return rand() % ((high + 1) - low) + low;
}

void draw_court(){
    ST7735_DrawLine(0, 0, 159, 0, ST7735_WHITE);
    ST7735_DrawLine(0, 0, 0, 120, ST7735_WHITE);
    ST7735_DrawLine(0, 120, 159, 120, ST7735_WHITE);
    ST7735_DrawLine(159, 120, 159, 0, ST7735_WHITE);
    ST7735_DrawLine(80, 0, 80, 120, ST7735_WHITE);
}

void end_of_game(int player){
   ST7735_FillScreen(ST7735_BLACK);
   while(1){
   
   ST7735_DrawLine(0, 0, 159, 0, ST7735_WHITE);
   ST7735_DrawLine(0, 0, 0, 120, ST7735_WHITE);
   ST7735_DrawLine(0, 120, 159, 120, ST7735_WHITE);
   ST7735_DrawLine(159, 120, 159, 0, ST7735_WHITE);
   if(player == 1){
    ST7735_DrawWString(35, 50, "Bot Win",  Font20, 1, ST7735_RED, ST7735_BLACK);
   }
   if(player == 2){
    ST7735_DrawWString(35, 50, "You Win",  Font20, 1, ST7735_GREEN, ST7735_BLACK);
   }
   }
   GameIsRunning = false;
   
}
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

#ifdef  USE_FULL_ASSERT
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

