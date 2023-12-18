#pragma once
#include <stdint.h>
#include "ADC.h"
#include "CalTable.h"
typedef struct 
{
	Temperature_tenthsC ntcs[NUMBER_ADCS];
}BankTemps;

void InitThermometer(CalTable);
void RefreshBanks(ADCReadings*);
void GetBankMins(ADCReadings*, Temperature_tenthsC*, Temperature_tenthsC*);