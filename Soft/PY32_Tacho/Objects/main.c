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
ADC_HandleTypeDef AdcHandle;
ADC_ChannelConfTypeDef AdcChanConf;

enum state current_state = rev;
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);

uint8_t buffer[BUFFER_L] = {0};

void display_revolutions(uint16_t revolutions)
{
	uint8_t pos = 3;
	buffer[0] = 'M';
	buffer[1] = 'P';
	buffer[2] = 'R';
	while ((revolutions > 0)&&(pos<BUFFER_L-1))
	{
		buffer[pos] = '0'+ revolutions%10;
		revolutions /= 10;
		pos++;
	}
	buffer[pos] = 0;
}

void display_minimum(int16_t delta)
{
	uint8_t pos = 3;
	uint8_t sign = '+';
	
	if (delta < 0)
	{
		sign = '-';
		delta = -delta;
	}
	buffer[0] = 'M';
	buffer[1] = 'P';
	buffer[2] = 'R';
	do
	{
		buffer[pos] = '0'+ delta%10;
		delta /= 10;
		pos++;
	}
	while ((delta != 0)&&(pos<BUFFER_L-2));
	buffer[pos] = sign;
	buffer[pos+1] = 0;
}

void display_temperature(int16_t degrees) //0.1 fixed dot
{
	uint8_t pos = 4;
	uint8_t sign = '+';
	
	if (degrees < 0) 
	{
		sign = '-';
		degrees = -degrees;
	}
	buffer[0] = 'C';
	buffer[1] = '`';
	buffer[2] = '0'+ degrees%10;
	degrees /= 10;
	buffer[3] = '.';
	do
	{
		buffer[pos] = '0'+ degrees%10;
		degrees /= 10;
		pos++;
	}
	while ((degrees != 0)&&(pos<BUFFER_L-2));
	buffer[pos] = sign;
	buffer[pos+1] = 0;
}

void display_voltage(uint16_t millivolts)
{
	uint8_t pos = 1;
	buffer[0] = 'V';
	while (pos < 4)
	{
		buffer[pos] = '0'+ millivolts%10;
		millivolts /= 10;
		pos++;
	}
	buffer[pos] = '.';
	pos++;
	do
	{
		buffer[pos] = '0'+ millivolts%10;
		millivolts /= 10;
		pos++;
	}
	while ((millivolts > 0)&&(pos<BUFFER_L-2));
	buffer[pos] = '+';
	buffer[pos+1] = 0;
}

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
	ADCInit();
	timerDisplayInit();
	timerMeasureInit();
  
  /* Infinite loop */
  while (1)
  {
		//if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
		//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		//else			
		//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
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
  
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;//GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_SET);
}

void ADCInit(void)
{	
	//Enable Clocks
	__HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();
	__HAL_RCC_ADC_CLK_ENABLE();
	
	//Start calibration
	AdcHandle.Instance = ADC1;
	
	if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)
	{
		APP_ErrorHandler();
	}
	
	//Populate ADC init data
	AdcHandle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_HSI_DIV1; //Full HSI speed
	AdcHandle.Init.Resolution = ADC_RESOLUTION_12B; //12 bits
	AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT; //Right aligned
	AdcHandle.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD; //Don't plan to use
	AdcHandle.Init.EOCSelection = ADC_EOC_SINGLE_CONV; //single conversion
	AdcHandle.Init.LowPowerAutoWait = DISABLE; //use all the power
	AdcHandle.Init.ContinuousConvMode = DISABLE; //don't need for polling
	AdcHandle.Init.DiscontinuousConvMode = DISABLE; //don't need for polling
	AdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START; //Will start ADC in code
	AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; //not going to use external trigger
	//AdcHandle.Init.DMAContinuousRequests = DISABLE; //Not using DMA
	AdcHandle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN; //shouldn't matter for polling
	AdcHandle.Init.SamplingTimeCommon = ADC_SAMPLETIME_239CYCLES_5; //Setting conversion time to 41.5 cycles
	
	//Initialize ADC
	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		APP_ErrorHandler();
	}
	
  ADC->CCR |= ADC_CCR_ALL; //Enable both reference and temperature sensor
	
	//Now set ADC rank and channel
	AdcChanConf.Rank = 0; //highest rank, only using one channel
	AdcChanConf.Channel = ADC_CHANNEL_VREFINT;
	
	if (HAL_ADC_ConfigChannel(&AdcHandle, &AdcChanConf) != HAL_OK)
	{
		APP_ErrorHandler();
	}
}


void timerMeasureInit(void)
{
	/* 	Trigger 4x per second, or every 250ms
			Clock at 24MHz -> 24 million cycles
			prescaler set to 24
			1 count equals 1uS*/
	TIM_SlaveConfigTypeDef tim1SlaveHandler;
	TIM_IC_InitTypeDef periodChannelHandler;
	TIM_IC_InitTypeDef dutyChannelHandler;
	
	tim1MeasureHandle.Instance = TIM1;																						//Timer 1 advanced timer
  tim1MeasureHandle.Init.Period            = 50000 - 1;														//Timer count = (period+1)*(prescaler+1)
  tim1MeasureHandle.Init.Prescaler         = 24 - 1;
  tim1MeasureHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;						//Use full clock rate
  tim1MeasureHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim1MeasureHandle.Init.RepetitionCounter = 1 - 1;
  tim1MeasureHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	
  if (HAL_TIM_IC_Init(&tim1MeasureHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	tim1SlaveHandler.SlaveMode = TIM_SLAVEMODE_RESET;
	tim1SlaveHandler.InputTrigger = TIM_TS_TI1FP1;
	tim1SlaveHandler.TriggerPolarity = TIM_TRIGGERPOLARITY_FALLING;
	if (HAL_TIM_SlaveConfigSynchro_IT(&tim1MeasureHandle, &tim1SlaveHandler) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	periodChannelHandler.ICPolarity = TIM_ICPOLARITY_FALLING;
	periodChannelHandler.ICSelection = TIM_ICSELECTION_DIRECTTI;
	if (HAL_TIM_IC_ConfigChannel(&tim1MeasureHandle, &periodChannelHandler, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

	dutyChannelHandler.ICPolarity = TIM_ICPOLARITY_RISING;
	dutyChannelHandler.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	if (HAL_TIM_IC_ConfigChannel(&tim1MeasureHandle, &dutyChannelHandler, TIM_CHANNEL_2) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	TIM_CCxChannelCmd(tim1MeasureHandle.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);
	
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
  tim16DisplayHandle.Init.Period            = 800 - 1;												//Timer count = (period+1)*(prescaler+1)
  tim16DisplayHandle.Init.Prescaler         = 0;
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
	
  //if (HAL_TIM_Base_Start_IT(&tim16DisplayHandle) != HAL_OK)
  //{
  //  APP_ErrorHandler();
  //}
}

void HAL_TIM_TriggerCallback(TIM_HandleTypeDef *htim)
{
	static uint16_t delay = 0;
	static uint16_t minimum = 0;
	static uint32_t accumulator = 0;
	uint16_t curr_period = HAL_TIM_ReadCapturedValue(&tim1MeasureHandle, TIM_CHANNEL_1);
	uint16_t curr_duty = HAL_TIM_ReadCapturedValue(&tim1MeasureHandle, TIM_CHANNEL_2);
	if ((curr_duty > curr_period>>2)&&(curr_duty < (curr_period>>2)*3))
	{
		tim16DisplayHandle.Instance->ARR = (uint32_t)curr_period>>2 ;
		if (curr_duty < (curr_period>>4)*6)
		{
			current_state = rev;
			display_revolutions((uint16_t)(10000000/(curr_period>>1))); //60M microseconds/3 poles/2 for stability
		}
		else if (curr_duty < (curr_period>>4)*7)
		{
			if (current_state != min) 
			{
				buffer[0] = 'T';
				buffer[1] = 'I';
				buffer[2] = 'A';
				buffer[3] = 'W';
				buffer[4] = 0;
				delay = 0;
				minimum = 0;
				accumulator = 0;
			}
			current_state = min;
			if (delay < 255) 
			{
				accumulator += curr_period>>1;
				delay++;
			}
			else if (delay < 256) 
			{
				accumulator = 10000000*delay/accumulator; //Get average RPM
				minimum = accumulator; //Start with calculated average
				delay++;
			}
			else
			{
				if ((10000000/(curr_period>>1)) < minimum) minimum = (10000000/(curr_period>>1));
				display_minimum((int16_t)minimum-accumulator);
			}
		}
		else if (curr_duty < (curr_period>>4)*8)
		{
			if (current_state != temp) 
			{
				AdcChanConf.Channel = ADC_CHANNEL_TEMPSENSOR;
				if (HAL_ADC_ConfigChannel(&AdcHandle, &AdcChanConf) != HAL_OK) { APP_ErrorHandler(); }
				HAL_ADC_Start(&AdcHandle);
				current_state = temp;
				return;
			}
			HAL_ADC_PollForConversion(&AdcHandle, 1000);
			display_temperature((int16_t)((85.0-30.0)*(HAL_ADC_GetValue(&AdcHandle)-HAL_ADC_TSCAL1)/(HAL_ADC_TSCAL2-HAL_ADC_TSCAL1)+30.0));
			HAL_ADC_Start(&AdcHandle);
		}
		else
		{
			if (current_state != volt) 
			{
				AdcChanConf.Channel = ADC_CHANNEL_VREFINT;
				if (HAL_ADC_ConfigChannel(&AdcHandle, &AdcChanConf) != HAL_OK) { APP_ErrorHandler(); }
				HAL_ADC_Start(&AdcHandle);
				current_state = volt;
				return;
			}
			HAL_ADC_PollForConversion(&AdcHandle, 1000);
			display_voltage((uint16_t)(1200*4095/HAL_ADC_GetValue(&AdcHandle)));
			HAL_ADC_Start(&AdcHandle);
		}
	//curr_duty = 0xFFFF;
	
		if (HAL_TIM_Base_Start_IT(&tim16DisplayHandle) != HAL_OK)  { APP_ErrorHandler(); }
	}
	//prev_duty = curr_duty;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t sym = 0;
  static uint16_t col = 0;
	//HAL_GPIO_WritePin(GPIOA, /*counter*/TachoFont[counter], GPIO_PIN_SET);
	//counter++;
	//if( counter > 60) counter = 0;
	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);//|GPIO_PIN_2|GPIO_PIN_3);
	
	if(buffer[sym])
	{
		GPIOA->ODR = (GPIOA->ODR&(~0x1F))|TachoFont[(buffer[sym]-' ')*6+col];
		//HAL_GPIO_WritePin(GPIOA, 0x1F, GPIO_PIN_RESET);
		//HAL_GPIO_WritePin(GPIOA, /*counterTacho*/Font[counter], GPIO_PIN_SET);
		col++;
		if (col>5) 
		{
			sym++;
			col = 0;
		}
		if (HAL_TIM_Base_Start_IT(&tim16DisplayHandle) != HAL_OK) { APP_ErrorHandler(); }
	}
	else 
	{
		GPIOA->ODR |= 0x1F;
		sym = 0;
		col = 0;
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
