#include "I2C.h"
#include "stm32f0xx_hal.h"
#include "PinDefs.h"
#include <stdbool.h>

I2C_HandleTypeDef hi2c;

static bool FakeSemaphore = false;


void I2C1_IRQHandler()
{
	const uint32_t errFlags = I2C_FLAG_BERR | I2C_FLAG_PECERR | I2C_FLAG_OVR | I2C_FLAG_ARLO | I2C_FLAG_TIMEOUT | I2C_FLAG_BUSY;
	uint8_t err = __HAL_I2C_GET_FLAG(&hi2c, errFlags);
	if (err)
	{
		HAL_I2C_ER_IRQHandler(&hi2c);
		return;
	}
	HAL_I2C_EV_IRQHandler(&hi2c);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	FakeSemaphore = true;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	FakeSemaphore = true;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	FakeSemaphore = true;
}

void InitI2C()
{
	GPIO_InitTypeDef init;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	init.Mode = GPIO_MODE_AF_OD;
	init.Pull = GPIO_NOPULL;
	init.Alternate = GPIO_AF1_I2C1;   
	
	init.Pin = I2C_SCL.pinNumber;
	HAL_GPIO_Init(I2C_SCL.pinPort, &init);
	init.Pin = I2C_SDA.pinNumber;
	HAL_GPIO_Init(I2C_SDA.pinPort, &init);
	__HAL_RCC_I2C1_CLK_ENABLE();
	
	
	hi2c.Instance = I2C1;
	hi2c.Init.Timing = 0x0000DDDE;
	hi2c.Init.OwnAddress1 = 0;
	hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c.Init.OwnAddress2 = 0;
	hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c) != HAL_OK)
	{
		//Error_Handler();
	}
	
	//NVIC_SetPriority(I2C1_IRQn, 0x07);
	//NVIC_EnableIRQ(I2C1_IRQn);
}

void WriteI2C(uint8_t address, uint8_t* buffer, uint8_t len)
{
	HAL_I2C_Master_Transmit(&hi2c, (uint16_t)address, buffer, len, 1000);
}

void ReadI2C(uint8_t address, uint8_t* buffer, uint8_t len)
{
	HAL_I2C_Master_Receive(&hi2c, (uint16_t)address, buffer, len, 1000);
}

void WriteToAddress(uint8_t address, uint8_t memAddress, uint8_t* writeDat, uint8_t writeLen)
{
	HAL_I2C_Mem_Write(&hi2c, address, memAddress, 1, writeDat, writeLen, 1000);
}

void ReadFromAddress(uint8_t address, uint8_t memAddress, uint8_t* readDat, uint8_t readLen)
{
	HAL_I2C_Mem_Read(&hi2c, address, memAddress, 1, readDat, readLen, 1000); 
}
