#pragma once

#include <memory>
#include <FreeRTOS.h>
#include <Wire.h>
#include "devices/TLC59108.h"
#include "devices/PCF8574.h"

using namespace std;

class HBI {
    private:
        shared_ptr<TwoWire> i2c;
        void HBIInterruptISR();
    public:
        HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema);
        ~HBI();
        void start();
        void enableVegas();
        void disableVegas();
        SemaphoreHandle_t i2cSema;
        unique_ptr<TLC59108> ledDriver1;
        unique_ptr<TLC59108> ledDriver2;
        unique_ptr<TLC59108> ledDriver3;
        unique_ptr<PCF8574> ioExpander1;
        unique_ptr<PCF8574> ioExpander2;
        unique_ptr<PCF8574> ioExpander3;
};
