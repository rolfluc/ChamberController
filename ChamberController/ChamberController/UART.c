#include "UART.h"
#include <string.h>
#include "PinDefs.h"
#include <../CMSIS_RTOS/cmsis_os.h>
#include "EEPROM.h"

static const uint32_t uartPinMode = GPIO_MODE_AF_PP;
static const uint32_t uartPinPull = GPIO_NOPULL;
static const uint32_t uartPinSpeed = GPIO_SPEED_FREQ_HIGH;
UART_HandleTypeDef UartPort;

extern Temperature_tenthsC setTemp;
extern Temperature_tenthsC tempRange;

#define BUFFER_LEN 32
uint8_t writeBuffer[BUFFER_LEN];
uint8_t readBuffer[BUFFER_LEN];
uint8_t bufferPtr = 0;

typedef enum
{
	CommandStart = 1,
	CommandStop = 2,
	CommandSetpoint = 3,
	CommandBand = 4,
	CommandNone = 0
}Commands;

typedef enum
{
	Running,
	Stopped,
}UartStateMachine;

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
	UartPort.Init.BaudRate = 115200;
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
}

HAL_StatusTypeDef WriteBlocking(uint8_t* buffer, uint8_t len)
{
	return HAL_UART_Transmit(&UartPort, buffer, len, 1000);
}

HAL_StatusTypeDef ReadBlocking(uint8_t* buffer, uint8_t len)
{
	return HAL_UART_Receive(&UartPort, buffer, len, 10);
}

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


void ReadNonblocking(uint8_t* buffer, uint8_t len)
{
	HAL_UART_Receive(&UartPort, readBuffer, 1, 0xffffffff);
}

static void ProcessCommand(uint8_t input0, uint8_t input1, uint8_t input2, uint8_t input3)
{
	int16_t setpoint = 0;
	int16_t band = 0;
	static UartStateMachine currentState = Stopped;
	static const uint8_t asciiSubtract = 48;
	input0 -= asciiSubtract;
	if (input1 != '-')
	{
		input1 -= asciiSubtract;
	}
	input2 -= asciiSubtract;
	input3 -= asciiSubtract;
	Commands foundCmd = CommandNone;
	switch ((Commands)input0)
	{
		case CommandStart:
		{
			currentState = Running;
		}
		break;
		case CommandStop:
		{
			currentState = Stopped;
		}
		break;
		case CommandSetpoint:
		{
			if (currentState == Stopped)
			{
				return;
			}
			if (input1 == '-')
			{
				setpoint = input2 * 10 + input3;
				setpoint *= -1;
			}
			else
			{
				setpoint = input1 * 100 + input2 * 10 + input3;
			}
			SetStoredTemp((Temperature_tenthsC)setpoint) ;
			getAndValidateTemps();
		}
		break;
		case CommandBand:
		{
			if (currentState == Stopped)
			{
				return;
			}
			if (input1 == '-')
			{
				// TODO ignore, and just use the other two.
				band = input2 * 10 + input3;
			}
			else
			{
				band = input1 * 100 + input2 * 10 + input3;
			}
			SetStoredTempRange((uint8_t)band);
			getAndValidateTemps();
		}
		break;
	default:
		break;
	}
}

void RunUARTTask()
{
	uint8_t backReturn = '\b';
	HAL_StatusTypeDef ret;
	static const uint32_t uartByteRate = 90;
	for (;;)
	{
		ret = ReadBlocking(&readBuffer[bufferPtr], 1);
		if (ret == HAL_OK)
		{
			WriteBlocking(&readBuffer[bufferPtr], 1);
			if ((readBuffer[bufferPtr] < 48 || readBuffer[bufferPtr] > 57) && readBuffer[bufferPtr] != '-')
			{
				WriteBlocking(&backReturn, 1);	
			}
			else
			{
				bufferPtr++;
			}
			if (bufferPtr == 4)
			{
				ProcessCommand(readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
				bufferPtr = 0;
			}
		}
		osDelay(uartByteRate);
	}			
}