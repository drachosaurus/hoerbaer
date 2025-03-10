#pragma once

#include <memory>
#include <Wire.h>

using namespace std;

class Power {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
        bool isCharging();
    public:
        Power(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema);
        void disableVCCPowerSave();
        void enableVCCPowerSave();
        void enableAudioVoltage();
        void disableAudioVoltage();
        void initializeChargerAndGauge();
        void setGaugeToSleep();
        bool checkBatteryShutdown();
        bool checkBatteryShutdownLoop();
};
