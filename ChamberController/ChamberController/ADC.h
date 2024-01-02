#pragma once
#include <stdint.h>
#include "SysDefs.h"
typedef struct 
{
	uint16_t adcCounts[NUMBER_ADCS];
}ADCReadings;

void InitADC();
ADCReadings RefreshReadings();
uint16_t CountsToPercentage(uint16_t count);
