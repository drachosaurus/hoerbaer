#pragma once

#include <FreeRTOS.h>
#include <semphr.h>
#include <memory>
#include <Wire.h>

using namespace std;

class Utils {
    public:
        static void scanI2CBus(shared_ptr<TwoWire> wire);
        static uint8_t writeI2CRegister(shared_ptr<TwoWire> wire, uint8_t address, uint8_t reg, uint8_t value);
        static uint8_t readI2CRegister(shared_ptr<TwoWire> wire, uint8_t address, uint8_t reg, uint8_t* buffer, uint8_t length);
};