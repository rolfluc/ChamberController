#pragma once
#include <stdint.h>
#include "SysDefs.h"
#include "Temperature.h"
typedef struct
{
	Temperature_tenthsC offset;
	uint16_t multiplier_thousandths;
}Calibration;

typedef struct 
{
	Calibration bank[NUMBER_ADCS];
}CalTable;

CalTable AcquireCal();