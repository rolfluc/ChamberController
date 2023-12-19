#pragma once
#include <stdbool.h>

void InitRelays();

typedef enum 
{
	Heater = 0,
	Cooler
}Relay;

typedef enum
{
	Relay_Off = 0,
	Relay_On,
}RelayState;

void SetRelay(Relay, RelayState);
RelayState GetRelayState(Relay);