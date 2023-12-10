#include "log.h"
#include "utils.h"

void Utils::scanI2CBus(std::shared_ptr<TwoWire> wire) 
{
    Log::println("UTIL", "Scanning I2C bus...");
    for (uint8_t address = 1; address < 127; address++) 
    {
        wire->beginTransmission(address);
        uint8_t error = wire->endTransmission();
        if (error == 0) 
            Log::println("UTIL", "I2C device found at address 0x%02x (R: 0x%02x, W: 0x%02x)", address, (address << 1) | 1, (address << 1));
        else if (error == 4) 
            Log::println("UTIL", "Unkonwn error at address 0x%02x", address);
    }
    Log::println("UTIL", "Done scanning I2C bus.");
}

uint8_t Utils::writeI2CRegister(shared_ptr<TwoWire> wire, uint8_t address, uint8_t reg, uint8_t value)
{
    wire->beginTransmission(address);
    wire->write(reg);
    wire->write(value);
    return wire->endTransmission();
}

uint8_t Utils::readI2CRegister(shared_ptr<TwoWire> wire, uint8_t address, uint8_t reg, uint8_t *buffer, uint8_t length)
{
    wire->beginTransmission(address);
    wire->write(reg);
    wire->endTransmission(false);
    wire->requestFrom(address, length);
    wire->readBytes(buffer, length);
    return wire->endTransmission();
}
