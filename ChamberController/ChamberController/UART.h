#pragma once
#include <stdint.h>
void InitUart();

void WriteBlocking(uint8_t* buffer, uint8_t len);
void ReadBlocking(uint8_t* buffer, uint8_t len);
void WriteNonblocking(uint8_t* buffer, uint8_t len);
void ReadNonblocking(uint8_t* buffer, uint8_t len);