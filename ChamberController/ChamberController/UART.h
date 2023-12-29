#pragma once
#include <stdint.h>
#include <stm32f0xx_hal.h>
void InitUart();

HAL_StatusTypeDef WriteBlocking(uint8_t* buffer, uint8_t len);
HAL_StatusTypeDef ReadBlocking(uint8_t* buffer, uint8_t len);
void WriteNonblocking(uint8_t* buffer, uint8_t len);
void ReadNonblocking(uint8_t* buffer, uint8_t len);

void RunUARTTask();