#pragma once
#include "CalTable.h"

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

