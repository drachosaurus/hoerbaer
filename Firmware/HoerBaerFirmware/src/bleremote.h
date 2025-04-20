#pragma once

#include <BLEServer.h>
#include <memory>
#include <Wire.h>
#include "userconfig.h"
#include "power.h"
#include "audioplayer.h"

using namespace std;

class BLERemote {
    private:
        shared_ptr<UserConfig> userConfig;
        shared_ptr<Power> power;
        shared_ptr<AudioPlayer> audioPlayer;
        TickType_t lastCharacteristicsUpdate;
        BLEServer* bleServer;
        BLEService* bleService;
        BLECharacteristic* powerCharacteristic;
        BLECharacteristic* playerCharacteristic;
    public:
        BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power, shared_ptr<AudioPlayer> audioPlayer);
        void initialize();
        void bleRemoteLoop();
        void updateWifiState(bool connected, int32_t ipV4);
        void shutdown();
};
