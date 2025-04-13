#pragma once

#include <BLEServer.h>
#include <memory>
#include <Wire.h>
#include "userconfig.h"
#include "power.h"

using namespace std;

class BLERemote {
    private:
        shared_ptr<UserConfig> userConfig;
        shared_ptr<Power> power;
        TickType_t lastCharacteristicsUpdate;
        BLEServer* bleServer;
        BLEService* bleService;
        BLECharacteristic* powerCharacteristic;
        BLECharacteristic* playerCharacteristic;
    public:
        BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power);
        void initialize();
        void bleRemoteLoop();
        void shutdown();
};
