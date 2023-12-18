#include "LED.h"
#include "stm32f0xx_hal.h"
#include "PinDefs.h"

void InitLEDs()
{
	GPIO_InitTypeDef init;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	
	init.Pin = LED_Y.pinNumber;
	HAL_GPIO_Init(LED_Y.pinPort, &init);
	init.Pin = LED_R.pinNumber;
	HAL_GPIO_Init(LED_R.pinPort, &init);
	init.Pin = LED_G.pinNumber;
	HAL_GPIO_Init(LED_G.pinPort, &init);
}

void DriveLED(LED led, LEDState state)
{
	PinDef* pin;
	switch (led)
	{
	case Yellow:
		{
			pin = &LED_Y;
		}
		break;
	case Red:
		{
			pin = &LED_R;
		}
		break;
	case Green:
		{
			pin = &LED_G;
		}
		break;
	default:
		break;
	}
	HAL_GPIO_WritePin(pin->pinPort, pin->pinNumber, state == LEDOn ? GPIO_PIN_SET : GPIO_PIN_RESET);
}