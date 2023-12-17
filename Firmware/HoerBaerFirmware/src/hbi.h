#pragma once

#include <memory>
#include <FreeRTOS.h>
#include <Wire.h>
#include "devices/TLC59108.h"
#include "devices/PCF8574.h"
#include "userconfig.h"

using namespace std;

class HBI {
    private:
        shared_ptr<TwoWire> i2c;
        shared_ptr<HBIConfig> hbiConfig;
        uint32_t lastKnownButtonMask;
        void HBIInterruptISR();
    public:
        HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<HBIConfig> hbiConfig);
        ~HBI();
        void start();
        void enableVegas();
        void disableVegas();
        void dispatchButtonInput(uint32_t buttonMask);
        // these ones must be public because they are called from the task callback
        SemaphoreHandle_t i2cSema;
        unique_ptr<TLC59108> ledDriver1;
        unique_ptr<TLC59108> ledDriver2;
        unique_ptr<TLC59108> ledDriver3;
        unique_ptr<PCF8574> ioExpander1;
        unique_ptr<PCF8574> ioExpander2;
        unique_ptr<PCF8574> ioExpander3;
        
        static constexpr std::string_view IoConfigDefault = "TODO";
};
