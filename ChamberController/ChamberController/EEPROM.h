#pragma once
#include "CalTable.h"
#include "Temperature.h"

typedef struct
{
	uint8_t Major;
	uint8_t Minor;
}SW_Version;

void InitEEPROM();

Calibration ReadCalTable();
void SetCal(Calibration);

SW_Version GetVersion();
void SetVersion(SW_Version);

Temperature_tenthsC GetStoredTargetTemp();
void SetStoredTemp(Temperature_tenthsC);

uint8_t GetStoredTempRange();
void SetStoredTempRange(uint8_t);


// TODO hacky 
void getAndValidateTemps();