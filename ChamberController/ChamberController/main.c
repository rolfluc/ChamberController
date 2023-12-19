#include <stm32f0xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "CalTable.h"
#include "Thermometer.h"
#include "EEPROM.h"
#include "RelayController.h"
#include "UART.h"

osThreadId TempControlTaskHandle, ExternalControlsTaskHandle, TempControlTaskHandle;

static void ThermometerTask(void const *argument);
static void ExternalControlsTask(void const *argument);
static void TempControlTask(void const *argument);

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
	//InitUart();
	InitEEPROM();
	SW_Version ver = GetVersion();
	CalTable cal = AcquireCal();
	InitThermometer(cal);
	

	osThreadDef(Thermo, ThermometerTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(Control, ExternalControlsTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadDef(TempC, TempControlTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	
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
	ADCReadings tmp = readings[0];
	for (uint8_t i = 1; i < READINGS_COUNT; i++)
	{
		for (uint8_t j = 0; j < NUMBER_ADCS; j++)
		{
			tmp.adcCounts[j] += readings->adcCounts[i];
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
	static const uint32_t tempRefreshRate_ms = 1000;
	(void) argument;
	
	uint8_t readingPtr = 0;
	
	for (;;)
	{
		RefreshBanks(&readingBuffer[readingPtr]);
		readingPtr = (readingPtr + 1) % READINGS_COUNT;
		// TODO move the data out.
		osDelay(tempRefreshRate_ms);
	}
}

static void ExternalControlsTask(void const *argument)
{
	(void) argument;
	// TODO just call UART task here.
  
	for (;;)
	{
		osDelay(200);
	}
}

static void TempControlTask(void const *argument)
{
	(void) argument;
  
	for (;;)
	{
		osDelay(200);
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
