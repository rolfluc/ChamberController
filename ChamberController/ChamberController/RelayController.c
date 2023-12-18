#include "RelayController.h"
#include "PinDefs.h"

typedef struct
{
	PinDef drive_m;
	PinDef drive_p;
	RelayState currentState;
}RelayController;


RelayController heater0; 
RelayController heater1; 
RelayController charger; 

static inline void DriveRelay(RelayController* relay, RelayState state)	
{
	//No need to do anything if already in a state.
	if (relay->currentState == state)
		return;
	// TODO might be backwards.
	if (state == Relay_On)
	{
		HAL_GPIO_WritePin(relay->drive_m.pinPort, relay->drive_m.pinNumber, GPIO_PIN_SET);
		HAL_GPIO_WritePin(relay->drive_p.pinPort, relay->drive_p.pinNumber, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(relay->drive_m.pinPort, relay->drive_m.pinNumber, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(relay->drive_p.pinPort, relay->drive_p.pinNumber, GPIO_PIN_SET);
	}
	// Stop driving, to save power / heat. Wait some ms to ensure relay fires.
	HAL_Delay(10);
	HAL_GPIO_WritePin(relay->drive_m.pinPort, relay->drive_m.pinNumber, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(relay->drive_p.pinPort, relay->drive_p.pinNumber, GPIO_PIN_RESET);
	relay->currentState = state;
}

void InitRelays()
{
	GPIO_InitTypeDef init;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	
	init.Pin = HRelay0_m.pinNumber;
	HAL_GPIO_Init(HRelay0_m.pinPort, &init);
	init.Pin = HRelay0_p.pinNumber;
	HAL_GPIO_Init(HRelay0_p.pinPort, &init);
	init.Pin = HRelay1_m.pinNumber;
	HAL_GPIO_Init(HRelay1_m.pinPort, &init);
	init.Pin = HRelay1_p.pinNumber;
	HAL_GPIO_Init(HRelay1_p.pinPort, &init);
	init.Pin = CRelay_m.pinNumber;
	HAL_GPIO_Init(CRelay_m.pinPort, &init);
	init.Pin = CRelay_p.pinNumber;
	HAL_GPIO_Init(CRelay_p.pinPort, &init);
	
	//Keep Relays off on boot.
	heater0.drive_m = HRelay0_m;
	heater0.drive_p = HRelay0_p;
	DriveRelay(&heater0, Relay_Off);
	
	heater1.drive_m = HRelay1_m;
	heater1.drive_p = HRelay1_p;
	DriveRelay(&heater1, Relay_Off);
	
	charger.drive_m = CRelay_m;
	charger.drive_p = CRelay_p;
	DriveRelay(&charger, Relay_Off);
}



void SetRelay(Relay which, RelayState state)
{
	switch (which)
	{
	case Heater0:
		{
			DriveRelay(&heater0, state);
		}
		break;
	case Heater1:
		{
			DriveRelay(&heater1, state);
		}
		break;
	case Charger:
		{
			DriveRelay(&charger, state);
		}
		break;
	default:
		break;
	}
}

RelayState GetRelayState(Relay relay)
{
	switch (relay)
	{
	case Heater0:
		{
			return heater0.currentState;
		}
		break;
	case Heater1:
		{
			return heater1.currentState;
		}
		break;
	case Charger:
		{
			return charger.currentState;
		}
		break;
	default:
		break;
	}
}