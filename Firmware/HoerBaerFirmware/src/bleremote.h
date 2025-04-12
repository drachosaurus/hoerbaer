#pragma once

#include <BLEServer.h>
#include <memory>
#include <Wire.h>
#include "userconfig.h"

using namespace std;

class BLERemote {
    private:
        shared_ptr<UserConfig> userConfig;
        BLEServer* bleServer;
        BLEService* bleService;
        BLECharacteristic* powerCharacteristic;
        BLECharacteristic* playerCharacteristic;
    public:
        BLERemote(shared_ptr<UserConfig> userConfig);
        void initialize();
        void updateCharacteristics();
        void shutdown();
};
