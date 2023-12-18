#pragma once
#include "Temperature.h"
typedef struct
{
	Temperature_tenthsC temp;
	uint16_t Resistance_ohms;
}NTC;
// https://tools.molex.com/pdm_docs/sd/2152723107_sd.pdf

#define NUMBER_OF_NTC_TABLE 25
__attribute__((section(".FLASH"), used)) 
static const NTC TempTable[NUMBER_OF_NTC_TABLE] = {
	{ .temp = -400, .Resistance_ohms = 33710 },
	{ .temp = -350, .Resistance_ohms = 24330 },
	{ .temp = -300, .Resistance_ohms = 17760 },
	{ .temp = -250, .Resistance_ohms = 13100 },
	{ .temp = -200, .Resistance_ohms = 9766 },
	{ .temp = -150, .Resistance_ohms = 7326 },
	{ .temp = -100, .Resistance_ohms = 5550 },
	{ .temp = -50, .Resistance_ohms = 4242 },
	{ .temp = 0, .Resistance_ohms = 3271 },
	{ .temp = 50, .Resistance_ohms = 2541 },
	{ .temp = 100, .Resistance_ohms = 1999 },
	{ .temp = 150, .Resistance_ohms = 1571 },
	{ .temp = 200, .Resistance_ohms = 1249 },
	{ .temp = 250, .Resistance_ohms = 1000 },
	{ .temp = 300, .Resistance_ohms = 806 },
	{ .temp = 350, .Resistance_ohms = 654 },
	{ .temp = 400, .Resistance_ohms = 533 },
	{ .temp = 450, .Resistance_ohms = 438 },
	{ .temp = 500, .Resistance_ohms = 361 },
	{ .temp = 550, .Resistance_ohms = 300 },
	{ .temp = 600, .Resistance_ohms = 250 },
	{ .temp = 650, .Resistance_ohms = 210 },
	{ .temp = 700, .Resistance_ohms = 177 },
	{ .temp = 750, .Resistance_ohms = 150 },
	{ .temp = 1250, .Resistance_ohms = 5 }, //This entry is technically wrong. Just wanted a last ditch check.
};
