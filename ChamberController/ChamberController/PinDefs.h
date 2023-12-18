#pragma once
#include <stm32f0xx_hal.h>
typedef struct
{
	uint32_t pinNumber;
	GPIO_TypeDef* pinPort;
}PinDef;

static const PinDef NTC0 = { GPIO_PIN_0, GPIOA };
static const PinDef NTC1 = { GPIO_PIN_1, GPIOA };
static const PinDef NTC2 = { GPIO_PIN_2, GPIOA };
static const PinDef NTC3 = { GPIO_PIN_3, GPIOA };
static const PinDef HRelay0_m = { GPIO_PIN_4, GPIOB };
static const PinDef HRelay0_p = { GPIO_PIN_5, GPIOB };

static const PinDef HRelay1_m = { GPIO_PIN_0, GPIOF };
static const PinDef HRelay1_p = { GPIO_PIN_1, GPIOF };

static const PinDef CRelay_m = { GPIO_PIN_3, GPIOB };
static const PinDef CRelay_p = { GPIO_PIN_15, GPIOA };

static const PinDef UART_RX = { GPIO_PIN_9, GPIOA };
static const PinDef UART_TX = { GPIO_PIN_10, GPIOA };

static const PinDef UART1_RX = { GPIO_PIN_2, GPIOA };
static const PinDef UART1_TX = { GPIO_PIN_3, GPIOA };

static const PinDef I2C_SCL = { GPIO_PIN_6, GPIOB };
static const PinDef I2C_SDA = { GPIO_PIN_7, GPIOB };

static const PinDef LED_Y = { GPIO_PIN_0, GPIOB };
static const PinDef LED_R = { GPIO_PIN_1, GPIOB };
static const PinDef LED_G = { GPIO_PIN_8, GPIOA };