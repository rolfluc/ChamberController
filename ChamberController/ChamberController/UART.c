#include "UART.h"
#include <stm32f0xx_hal.h>
#include <string.h>
#include "PinDefs.h"

static const uint32_t uartPinMode = GPIO_MODE_AF_PP;
static const uint32_t uartPinPull = GPIO_NOPULL;
static const uint32_t uartPinSpeed = GPIO_SPEED_FREQ_HIGH;
UART_HandleTypeDef UartPort;

void InitUartPins()
{
	GPIO_InitTypeDef init;
	init.Speed = uartPinSpeed;
	init.Mode = uartPinMode;
	init.Pull = uartPinPull;
	//TODO USART port
	init.Alternate = GPIO_AF1_USART1;
	init.Pin = UART_TX.pinNumber;
	HAL_GPIO_Init(UART_TX.pinPort, &init);
	init.Pin = UART_RX.pinNumber;
	HAL_GPIO_Init(UART_RX.pinPort, &init);
}

void InitUart()
{
	InitUartPins();
	__HAL_RCC_USART1_CLK_ENABLE();
	UartPort.Instance = USART1;
	UartPort.Init.BaudRate = 38400;
	UartPort.Init.WordLength = UART_WORDLENGTH_8B;
	UartPort.Init.StopBits = UART_STOPBITS_1;
	UartPort.Init.Parity = UART_PARITY_NONE;
	UartPort.Init.Mode = UART_MODE_TX_RX;
	UartPort.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartPort.Init.OverSampling = UART_OVERSAMPLING_8;
	UartPort.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	if (HAL_UART_Init(&UartPort) != HAL_OK)
	{
		//TODO error
	}
	for (uint8_t i = 0; i < 8; i++)
	{
		char* tmp = "asdf";
		WriteBlocking((uint8_t*)tmp, 4);	
	}
}

void WriteBlocking(uint8_t* buffer, uint8_t len)
{
	HAL_UART_Transmit(&UartPort, buffer, len, 1000);
}

void ReadBlocking(uint8_t* buffer, uint8_t len)
{
	HAL_UART_Receive(&UartPort, buffer, len, 1000);
}

#define BUFFER_LEN 32
uint8_t writeBuffer[BUFFER_LEN];
void WriteNonblocking(uint8_t* buffer, uint8_t len)
{
	if (len > BUFFER_LEN)
	{
		//TODO ERROR
		return;
	}
	memcpy(writeBuffer, buffer, len);
	//TODO just IT? or DMA?
	HAL_UART_Transmit_IT(&UartPort, writeBuffer, len);
	
}

uint8_t readBuffer[BUFFER_LEN];
uint8_t bufferPtr = 0;
void ReadNonblocking(uint8_t* buffer, uint8_t len)
{
	HAL_UART_Receive(&UartPort, readBuffer, 1, 0xffffffff);
}
