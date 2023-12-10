#pragma once

#include <memory>
#include <Wire.h>

using namespace std;

class Power {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
    public:
        Power(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema);
        void DisableVCCPowerSave();
        void EnableVCCPowerSave();
        void EnableAudioVoltage();
        void DisableAudioVoltage();
        void InitializeChargerAndGauge();
        void CheckBatteryVoltage();
};
