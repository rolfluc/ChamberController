#include "EEPROM.h"
#include "I2C.h"

static const uint8_t PAGE_SIZE = 8;
static const uint8_t EEPROM_DEV_ADDR = 0xA0;
typedef enum
{
	SW_VER_MAJOR = 0,
	SW_VER_MINOR = 1,
	
}EEPROM_Address;

void InitEEPROM()
{
	InitI2C();
}

static void ByteWrite(uint8_t byte, uint8_t addr)
{
	uint8_t buffer[2] = { addr, byte };
	WriteI2C(EEPROM_DEV_ADDR, buffer, sizeof(buffer));
}

static void PageWrite(uint8_t* data, uint8_t len, uint8_t addr)
{
	if (len > PAGE_SIZE)
		// Error - data will overwrite.
		return;
	WriteI2C(addr, data, len);
}

static uint8_t ReadByte(uint8_t addr)
{
	uint8_t retVal = 0;
	ReadFromAddress(EEPROM_DEV_ADDR, addr, &retVal, 1);
	return retVal;
}

static void ReadBytes(uint8_t address, uint8_t* readDat, uint8_t readLen)
{
	ReadFromAddress(EEPROM_DEV_ADDR, address, readDat, readLen);
}

Calibration ReadCalTable()
{
	// TODO
	Calibration cal;
	return cal;
}

void SetCal(Calibration cal)
{
	// TODO
}

SW_Version GetVersion()
{
	SW_Version ver;
	uint8_t vals[2] = { 0, 0 };
	uint8_t verAddr = (uint8_t)SW_VER_MAJOR;
	ReadBytes(verAddr, vals, 2);
	ver.Major = vals[0];
	ver.Minor = vals[1];
	return ver;
}

void SetVersion(SW_Version ver)
{
	uint8_t vals[3] = { (uint8_t)SW_VER_MAJOR, ver.Major, ver.Minor };
	WriteI2C(EEPROM_DEV_ADDR, vals, 3);
}
