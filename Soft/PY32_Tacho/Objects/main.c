/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef tim1MeasureHandle;
TIM_HandleTypeDef tim16DisplayHandle;
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);

/**
  * @brief  Main program.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Systick. */
  HAL_Init();
  
  /* Configure the system clock */
  APP_SystemClockConfig(); 
	
	gpioInit();
	timerDisplayInit();
	//timerMeasureInit();
  
  /* Infinite loop */
  while (1)
  {
  }
}

void gpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_SET);
}


void timerMeasureInit(void)
{
	/* 	Trigger 4x per second, or every 250ms
			Clock at 24MHz -> 24 million cycles
			Period to 10,000, prescaler to 800
			10,000*800=8,000,000 */
	
	TIM_IC_InitTypeDef periodChannelHandler;
	TIM_IC_InitTypeDef dutyChannelHandler;
	
	tim1MeasureHandle.Instance = TIM1;																						//Timer 1 advanced timer
  tim1MeasureHandle.Init.Period            = 100 - 1;														//Timer count = (period+1)*(prescaler+1)
  tim1MeasureHandle.Init.Prescaler         = 24 - 1;
  tim1MeasureHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;						//Use full clock rate
  tim1MeasureHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim1MeasureHandle.Init.RepetitionCounter = 1 - 1;
  tim1MeasureHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	
  if (HAL_TIM_IC_Init(&tim1MeasureHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	if (HAL_TIM_IC_ConfigChannel(&tim1MeasureHandle, &periodChannelHandler, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

	if (HAL_TIM_IC_ConfigChannel(&tim1MeasureHandle, &dutyChannelHandler, TIM_CHANNEL_2) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
  if (HAL_TIM_IC_Start_IT(&tim1MeasureHandle, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

void timerDisplayInit(void)
{
	/* 	Trigger 4x per second, or every 250ms
			Clock at 24MHz -> 24 million cycles
			Period to 10,000, prescaler to 800
			10,000*800=8,000,000 */
	
	tim16DisplayHandle.Instance = TIM16;																						//Timer 1 advanced timer
  tim16DisplayHandle.Init.Period            = 100 - 1;												//Timer count = (period+1)*(prescaler+1)
  tim16DisplayHandle.Init.Prescaler         = 24 - 1;
  tim16DisplayHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;						//Use full clock rate
  tim16DisplayHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim16DisplayHandle.Init.RepetitionCounter = 1 - 1;
  tim16DisplayHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	
  if (HAL_TIM_Base_Init(&tim16DisplayHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
  /* Set the OPM Bit */
  tim16DisplayHandle.Instance->CR1 |= TIM_CR1_OPM;
	
  if (HAL_TIM_Base_Start_IT(&tim16DisplayHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

void HAL_TIM_TriggerCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
	  
  if (HAL_TIM_Base_Start_IT(&tim16DisplayHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
  */
static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Oscillator Configuration */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI; /* Select oscillators HSE,HSI,LSI */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* Enable HSI */
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* HSI not divided */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;  /* Configure HSI clock as 24MHz */
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                         /* Disable HSE */
  /*RCC_OscInitStruct.HSEFreq = RCC_HSE_16_24MHz;*/
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                         /* Disable LSI */

  /* Configure oscillators */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* Clock source configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* Select clock types HCLK, SYSCLK, PCLK1 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; /* Select HSI as the system clock */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     /* AHB clock not divide */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      /* APB clock not divided */
  /* Configure clock source */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
  /* Infinite loop */
  while (1)
  {
  }
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
  /* User can add his own implementation to report the file name and line number,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)  */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
