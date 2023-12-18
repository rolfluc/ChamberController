#pragma once
typedef enum
{
	Yellow,
	Red,
	Green,
}LED;
typedef enum 
{
	LEDOff,
	LEDOn,
}LEDState;
void InitLEDs();
void DriveLED(LED, LEDState);