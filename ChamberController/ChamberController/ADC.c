#include "ADC.h"
#include <stm32f0xx_hal.h>
#include "PinDefs.h"

//Assumes 12 bit
static const uint16_t ADC_MAX = 0x0fff;
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma;

void InitADC()
{
	ADC_ChannelConfTypeDef sConfig = { 0 };
	GPIO_InitTypeDef init;
	__HAL_RCC_ADC1_CLK_ENABLE();
	
	hadc.Instance = ADC1;
	hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc.Init.LowPowerAutoWait = DISABLE;
	hadc.Init.LowPowerAutoPowerOff = DISABLE;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	if (HAL_ADC_Init(&hadc) != HAL_OK)
	{
		//TODO fail
	}
	
	init.Mode = GPIO_MODE_ANALOG;
	init.Pull = GPIO_NOPULL;
	init.Pin = NTC0.pinNumber;
	HAL_GPIO_Init(NTC0.pinPort, &init);
	init.Pin = NTC1.pinNumber;
	HAL_GPIO_Init(NTC1.pinPort, &init);
}

ADCReadings RefreshReadings()
{
	ADCReadings readings;
	int i = 0;
	ADC_ChannelConfTypeDef sConfig = { 0 };
	sConfig.SamplingTime = ADC_SAMPLETIME_41CYCLES_5;
	
	for (uint32_t i = 0; i < NUMBER_ADCS; i++)
	{
		sConfig.Channel = ADC_CHANNEL_0+i;
		sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
		HAL_ADC_ConfigChannel(&hadc, &sConfig);
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, 10000);
		readings.adcCounts[i] = (uint16_t)HAL_ADC_GetValue(&hadc);
		HAL_ADC_Stop(&hadc);	
		sConfig.Rank = ADC_RANK_NONE;
		HAL_ADC_ConfigChannel(&hadc, &sConfig);
	}
	return readings;
}

//Returns in 10ths of a percent. e.g. 10.3% = 103
uint16_t CountsToPercentage(uint16_t count)
{
	// e.g. 0x0fff (4095 / 4095 = 100%)		4095 * 1000 = 4095000 / 4095 = 1000
	// e.g. 0x00ff (255 / 4095 = 6.227%)	255 * 1000 = 255000 / 4095 = 62
	// e.g. 0x000f (15 / 4095 = 0.366%)		15 * 1000 = 15000 / 4095 = 3
	uint32_t tmpCount = (count * 1000) / ADC_MAX;
	return tmpCount;
}