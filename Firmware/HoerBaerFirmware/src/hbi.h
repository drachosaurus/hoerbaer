#pragma once

#include <memory>
#include <FreeRTOS.h>
#include <Wire.h>
#include "devices/TLC59108.h"
#include "devices/PCF8574.h"
#include "userconfig.h"
#include "audioplayer.h"

using namespace std;

class HBI;

class HBI {
    private:
        shared_ptr<TwoWire> i2c;
        shared_ptr<HBIConfig> hbiConfig;
        uint32_t lastKnownButtonMask;
        void HBIInterruptISR();
        SemaphoreHandle_t i2cSema;
        unique_ptr<TLC59108> ledDriver1;
        unique_ptr<TLC59108> ledDriver2;
        unique_ptr<TLC59108> ledDriver3;
        unique_ptr<PCF8574> ioExpander1;
        unique_ptr<PCF8574> ioExpander2;
        unique_ptr<PCF8574> ioExpander3;
        shared_ptr<AudioPlayer> audioPlayer;
        void (*shutdownCallback)(void);
        void checkLongPressState();
        void setLedState();
        void dispatchButtonInput(uint32_t buttonMask);
        void dispatchEncoderButton(bool longPress);
    public:
        HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<HBIConfig> hbiConfig, shared_ptr<AudioPlayer> audioPlayer, void (*shutdownCallback)(void));
        ~HBI();
        void start();
        void runWorkerTask();
        void shutOffAllLeds();
        void waitUntilEncoderButtonReleased();
};
