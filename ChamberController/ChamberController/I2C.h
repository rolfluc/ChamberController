#pragma once
#include <stdint.h>
void InitI2C();
void WriteI2C(uint8_t, uint8_t*, uint8_t);
void ReadI2C(uint8_t, uint8_t*, uint8_t);
void ReadFromAddress(uint8_t address, uint8_t memAddress, uint8_t* readDat, uint8_t readLen);
void WriteToAddress(uint8_t address, uint8_t memAddress, uint8_t* writeDat, uint8_t writeLen);
