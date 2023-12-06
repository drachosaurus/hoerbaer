#pragma once

#include <memory>
#include <FreeRTOS.h>
#include <Wire.h>

using namespace std;

class HBI {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
        TaskHandle_t* listenerTaskHandle;
    public:
        HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema);
        void startInputListener();
};
