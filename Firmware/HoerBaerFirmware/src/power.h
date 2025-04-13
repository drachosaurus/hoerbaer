#pragma once

#include <memory>
#include <Wire.h>

using namespace std;

typedef struct {
    bool charging;
    float voltage;
    float percentage;
} PowerState;

class Power {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
        PowerState state;
        TickType_t lastBatteryCheck;
        bool isCharging();
        bool initialized;
    public:
        Power(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema);
        void disableVCCPowerSave();
        void enableVCCPowerSave();
        void enableAudioVoltage();
        void disableAudioVoltage();
        void initializeChargerAndGauge();
        void setGaugeToSleep();
        void updateState();
        bool checkBatteryShutdown();
        bool checkBatteryShutdownLoop();
        PowerState& getState();
};
