#include <stm32f0xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "CalTable.h"
#include "Thermometer.h"
#include "EEPROM.h"
#include "RelayController.h"
#include "UART.h"
#include <string.h> 

osThreadId TempControlTaskHandle, ExternalControlsTaskHandle, TempControlTaskHandle;

static void ThermometerTask(void const *argument);
static void ExternalControlsTask(void const *argument);
static void TempControlTask(void const *argument);

static const Temperature_tenthsC minTemp = -200;
static const Temperature_tenthsC maxTemp = 400;
static const Temperature_tenthsC defaultTemp = 200;
static const uint8_t defaultRange = 200;
static const uint8_t minRange = 50;

// When running a compressor, run for a minimum of this time.
static const uint64_t MinCompressorTime_ms = 1000 * 45;
// After finishing running the compressor, wait this time before starting again.
static const uint64_t TimeBetweenCompressor_ms = 1000 * 60;
// Do not allow to run for longer than 4 hours continuously without a quick break.
static const uint64_t CutoffTime_ms = 1000 * 60 * 60 * 4;

#define QUEUE_LENGTH 1
#define QUEUE_ITEM_LENGTH sizeof(ADCReadings)
static StaticQueue_t tempReadingsQueue;
uint8_t tempStorageArea[QUEUE_LENGTH * QUEUE_ITEM_LENGTH];
QueueHandle_t tempQueueHandle;

typedef enum
{
	Idling = 0,
	Cooling = 1,
	StartingCooling = 2,
	Heating = 3,
	StartingHeating = 4,
}TempStateMachine;

typedef enum 
{
	Temp_Less = 0,
	Temp_InBand = 1,
	Temp_Higher = 2,
}TempTarget;

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	{
	}
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
	static StaticTask_t xTimerTaskTCB;
	static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
	/* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

	/* Pass out the array that will be used as the Timer task's stack. */
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;

	/* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
	StackType_t **ppxIdleTaskStackBuffer,
	uint32_t *pulIdleTaskStackSize)
{
	/* If the buffers to be provided to the Idle task are declared inside this
	function then they must be declared static - otherwise they will be allocated on
	the stack and so not exists after this function exits. */
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static Temperature_tenthsC setTemp = defaultTemp;
static Temperature_tenthsC tempRange = defaultRange;

uint8_t temperatureWriteBuffer[10]; //Up to 4 characters, and 1 newline
void getAndValidateTemps()
{
	setTemp = GetStoredTargetTemp();
	// Really do not drive lower than -20C, or higher than 40C. 
	if (setTemp < minTemp || setTemp > maxTemp)
	{
		// Set to default if invalid read.
		setTemp = defaultTemp; 
	}
	tempRange = GetStoredTempRange();
	
	if (tempRange < minRange) 
	{
		tempRange = defaultRange;
	}
	// Now print out the details:
	// set:-300\r\n
	// bnd:200\r\n
	uint8_t writePtr = 0;
	temperatureWriteBuffer[writePtr++] = 's';
	temperatureWriteBuffer[writePtr++] = 'e';
	temperatureWriteBuffer[writePtr++] = 't';
	temperatureWriteBuffer[writePtr++] = ':';
	if (setTemp < 0)
	{
		temperatureWriteBuffer[writePtr++] = '-';
		setTemp *= -1;
	}
	temperatureWriteBuffer[writePtr++] = (setTemp / 100) + 48;
	temperatureWriteBuffer[writePtr++] = ((setTemp / 10) % 10) + 48;
	temperatureWriteBuffer[writePtr++] = (setTemp % 10) + 48;
	temperatureWriteBuffer[writePtr++] = '\r';
	temperatureWriteBuffer[writePtr] = '\n';
	WriteBlocking(temperatureWriteBuffer, writePtr);
	writePtr = 0;
	temperatureWriteBuffer[writePtr++] = 'b';
	temperatureWriteBuffer[writePtr++] = 'n';
	temperatureWriteBuffer[writePtr++] = 'd';
	temperatureWriteBuffer[writePtr++] = ':';
	if (tempRange < 0)
	{
		temperatureWriteBuffer[writePtr++] = '-';
		tempRange *= -1;
	}
	temperatureWriteBuffer[writePtr++] = (tempRange / 100) + 48;
	temperatureWriteBuffer[writePtr++] = ((tempRange / 10) % 10) + 48;
	temperatureWriteBuffer[writePtr++] = (tempRange % 10) + 48;
	temperatureWriteBuffer[writePtr++] = '\r';
	temperatureWriteBuffer[writePtr] = '\n';
	WriteBlocking(temperatureWriteBuffer, writePtr);
}

int main(void)
{
	HAL_Init();  
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOF_CLK_ENABLE();
	// Immediately turns off relays, as the boot state is unknown - latched relays
	InitRelays();
	
	SystemClock_Config();
	InitUart();
	InitEEPROM();
	SW_Version ver = GetVersion();
	CalTable cal = AcquireCal();
	InitThermometer(cal);
	getAndValidateTemps();
	
	osThreadDef(Thermo, ThermometerTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(Control, ExternalControlsTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(TempC, TempControlTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	
	tempQueueHandle = xQueueCreateStatic(QUEUE_LENGTH, QUEUE_ITEM_LENGTH, tempStorageArea, &tempReadingsQueue);
	
	TempControlTaskHandle = osThreadCreate(osThread(Thermo), NULL);
	ExternalControlsTaskHandle = osThreadCreate(osThread(Control), NULL);
	TempControlTaskHandle = osThreadCreate(osThread(TempC), NULL);
  
	// Start scheduler
	osKernelStart();

	 // We should never get here as control is now taken by the scheduler
	for (;;)
		;
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	osSystickHandler();
}

#define READINGS_COUNT 2
ADCReadings GetReadingsAverage(ADCReadings* readings)
{
	ADCReadings tmp;
	memset((void*)&tmp, 0, sizeof(ADCReadings));
	for (uint8_t i = 0; i < READINGS_COUNT; i++)
	{
		for (uint8_t j = 0; j < NUMBER_ADCS; j++)
		{
			tmp.adcCounts[j] += readings[i].adcCounts[j];
		}
	}
	
	for (uint8_t i = 0; i < NUMBER_ADCS; i++)
	{
		tmp.adcCounts[i] = tmp.adcCounts[i] / READINGS_COUNT;
	}
	
	return tmp;
}

static ADCReadings readingBuffer[READINGS_COUNT];
static void ThermometerTask(void const *argument)
{
	// Every second should be fine. Temperature moves slowly.
	static ADCReadings averageReadings;
	static const uint32_t tempRefreshRate_ms = 500;
	(void) argument;
	uint8_t readingPtr = 0;
	
	for (;;)
	{
		RefreshBanks(&readingBuffer[readingPtr]);
		readingPtr = (readingPtr + 1) % READINGS_COUNT;
		if (readingPtr == 0)
		{
			averageReadings = GetReadingsAverage(readingBuffer);
			Temperature_tenthsC averageTemp;
			GetAverageTemp(&averageReadings, &averageTemp);
			xQueueSend(tempQueueHandle, (void*)&averageTemp, 100);
			// No Snprintf because too little flash. Basic implementation.
			uint8_t writeTarget = 0;
			if (averageTemp < 0)
			{
				temperatureWriteBuffer[writeTarget++] = '-';
				averageTemp *= -1;
			}
			temperatureWriteBuffer[writeTarget++] = (averageTemp / 100) + 48;
			temperatureWriteBuffer[writeTarget++] = ((averageTemp / 10) % 10) + 48;
			temperatureWriteBuffer[writeTarget++] = (averageTemp % 10) + 48;
			temperatureWriteBuffer[writeTarget++] = '\r';
			temperatureWriteBuffer[writeTarget] = '\n';
			WriteBlocking(temperatureWriteBuffer, writeTarget);
		}
		osDelay(tempRefreshRate_ms);
	}
}

static void ExternalControlsTask(void const *argument)
{
	(void) argument;
	RunUARTTask();
}

static inline TempTarget IsInRange(Temperature_tenthsC newTemp)
{
	if ((newTemp < (setTemp + tempRange)) && (newTemp > (setTemp - tempRange)))
	{
		return Temp_InBand;	
	}
	else if (newTemp > setTemp + tempRange)
	{
		return Temp_Higher;
	}
	else
	{
		return Temp_Less;
	}
}

static inline bool shouldHeat(Temperature_tenthsC newTemp)
{
	return (newTemp < setTemp);
}

static inline bool shouldCool(Temperature_tenthsC newTemp)
{
	return (newTemp > setTemp);	
}

TempStateMachine currentState = Idling;
Temperature_tenthsC averageTemp;
uint64_t heatingStartTime = 0;
uint64_t coolingStartTime = 0;
uint64_t finishedCoolingTime = 0;

static void TempControlTask(void const *argument)
{
	(void) argument;
	for (;;)
	{
		xQueueReceive(tempQueueHandle, (void*)&averageTemp, 0xffffffff);
		switch (currentState)
		{
		case Idling:
			{
				TempTarget tgt = IsInRange(averageTemp);
				if (tgt == Temp_InBand) 
				{
					currentState = Idling;
				}
				else if (tgt == Temp_Higher)
				{					
					currentState = StartingCooling;
				}
				else
				{
					currentState = StartingHeating;
				}
			}
			break;
		case Cooling:
			{
				if (!shouldCool(averageTemp))
				{
					if (xTaskGetTickCount() > coolingStartTime + MinCompressorTime_ms)
					{
						//Turn off the compressor. Go back to idle.
						finishedCoolingTime = xTaskGetTickCount();
						SetRelay(Cooler, Relay_Off);
						currentState = Idling;
					}
					// If we're no longer needing to cool, but the compressor hasn't been on long enough, do nothing.
				}
				// If we've been heating for a long time.. just die off. 
				if (xTaskGetTickCount() > coolingStartTime + CutoffTime_ms)
				{
					SetRelay(Cooler, Relay_Off);
					SetRelay(Heater, Relay_Off);
					while (1) ;
				}
			}
			break;
		case Heating:
			{
				if (!shouldHeat(averageTemp)) 
				{
					SetRelay(Heater, Relay_Off);
					currentState = Idling;
				}
				// If we've been heating for a long time.. just die off. 
				if (xTaskGetTickCount() > heatingStartTime + CutoffTime_ms)
				{
					SetRelay(Cooler, Relay_Off);
					SetRelay(Heater, Relay_Off);
					while (1) ;
				}
			}
			break;
		case StartingHeating:
			{
				TempTarget tgt = IsInRange(averageTemp);
				// Double check before starting
				if (tgt == Temp_InBand || tgt == Temp_Higher) 
				{
					currentState = Idling;
					continue;
				}
				heatingStartTime = xTaskGetTickCount();
				SetRelay(Heater, Relay_On);
				currentState = Heating;
			}
			break;
		case StartingCooling:
			{
				TempTarget tgt = IsInRange(averageTemp);
				// Double check before starting
				if (tgt == Temp_InBand || tgt == Temp_Less) 
				{
					currentState = Idling;
					continue;
				}
				// After turning off the compressor, give it a minute to cool down  / ect. Do not want to constantly throttle the compressor.
				if (finishedCoolingTime == 0 || (xTaskGetTickCount() > finishedCoolingTime + TimeBetweenCompressor_ms))
				{
					coolingStartTime = xTaskGetTickCount();
					SetRelay(Cooler, Relay_On);
					currentState = Cooling;
				}
			}
			break;
		default:
			break;
		}
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
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
	while (1)
	{
	}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


void HardFault_Handler()
{
	NVIC_SystemReset();
}