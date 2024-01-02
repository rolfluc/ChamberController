#include "Thermometer.h"
#include "NTC.h"

CalTable table;

void InitThermometer(CalTable calcTable)
{
	InitADC();
	table = calcTable;
}
static const uint16_t Resistor1_ohm = 1000;

//Data for regression:
//, SW
//100, 80
//200, 183
//300, 283
//400, 379
//500, 474
//600, 572
//700, 650
//1000, 937
//1500, 1396
//2000, 1872
//2500, 2319
//3000, 2728
//3500, 3130
//4000, 3483
//5000, 4490
//6000, 5285
// y = 1.0734x - 8.4401
// y = (1073x + 8440) / 10000

uint32_t fitResistance(uint32_t res)
{
	// Manually modified to better fit.
	//return ((res * res * 16) - (16956 * res) + (6801869)) / 10000;
	return (res * 1073 - 8440) / 1000;
}

Temperature_tenthsC interpolatePosition(uint8_t pt1, uint8_t pt2, uint32_t resistance_ohms)
{
	Temperature_tenthsC after = TempTable[pt1].temp;
	if (pt1 == 0)
	{
		return after;
	}
	Temperature_tenthsC before = TempTable[pt2].temp;
	uint32_t delta = (TempTable[pt2].Resistance_ohms - TempTable[pt1].Resistance_ohms) / 10;
	for (uint8_t i = 0; i < 10; i++)
	{
		if (resistance_ohms < (TempTable[pt1].Resistance_ohms + delta*i))
		{
			// Assumption, each temp_C is seperated by 5C intervals, or 50 tenths. 
			// Dividing by 10, 5 tenths per step. 
			return (after - 5 * i);
		}
	}
	return after;
}

static inline Temperature_tenthsC ConvertCounts(uint16_t count, uint8_t index)
{
	//R1 = 1000
	//V = 3.3R2/(R1+R2)
	//V(R1 + R2) = 3.3R2
	//VR1 + VR2 = 3.3R2
	//
	//R2(3300 - V) = 1000V
	//R2 = 1000V / (3300 - V)
	//Actually 10ths of percent, and whole digit
	uint16_t percent = CountsToPercentage(count);
	//Multiplied by 3.3V (33)
	uint16_t miliVolts = (percent * 33) / 10;
	uint32_t Res2 = fitResistance((1000 * miliVolts) / (3300 - miliVolts));
	Temperature_tenthsC ret = 0;
	for (uint8_t i = 0; i < NUMBER_OF_NTC_TABLE; i++)
	{
		uint16_t res = TempTable[i].Resistance_ohms;
		if (res < Res2) 
		{
			ret = interpolatePosition(i, i - 1, Res2);
			break;
		}
	}
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
