#include "Thermometer.h"
#include "NTC.h"

CalTable table;

void InitThermometer(CalTable calcTable)
{
	InitADC();
	table = calcTable;
}
static const uint16_t Resistor1_ohm = 1000;

// 40k		3934
// 35k		3922
// 30k		3900
// 25k		3875
// 20k		3840
// 15k		3780
// 10k		3660
// 5k 		3360
// 2.5k 	2875
// 2k 		2675
// 1.5k 	2380
// 1.25k 	2230
// 1k 		2000
// 0.75k 	1700
// 0.5k 	1320
// 0.25k 	770

static inline Temperature_tenthsC ConvertCounts(uint16_t count, uint8_t index)
{
	//R1 = 1000
	//V = 3.3R2/(R1+R2)
	//V(R1 + R2) = 3.3R2
	//VR1 + VR2 = 3.3R2
	//R2(3300 - V) = 1000V
	//R2 = 1000V / (3300 - V)
	//Actually 10ths of percent, and whole digit
	uint16_t percent = CountsToPercentage(count);
	//Multiplied by 3.3V (33)
	uint16_t miliVolts = (percent * 33) / 10;
	uint32_t Res2 = (1000 * miliVolts) / (3300 - miliVolts);
	Temperature_tenthsC ret = 0;
	for (uint8_t i = 0; i < NUMBER_OF_NTC_TABLE; i++)
	{
		uint16_t res = TempTable[i].Resistance_ohms;
		if (res < Res2) 
		{
			ret = TempTable[i].temp;
			break;
		}
	}
	// TODO calibration table? 
	return ret;
}

void RefreshBanks(ADCReadings* temps)
{
	*temps = RefreshReadings();
}

void GetAverageTemp(ADCReadings* adcReadings, Temperature_tenthsC* tempOut)
{
	Temperature_tenthsC outTemp = 0;
	for (uint8_t i = 0; i < NUMBER_ADCS; i++)
	{
		outTemp += ConvertCounts(adcReadings->adcCounts[i],i);
	}
	*tempOut = outTemp / NUMBER_ADCS;
}
