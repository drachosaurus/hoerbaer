#pragma once

#include <BLEServer.h>
#include <memory>
#include <Wire.h>
#include "userconfig.h"
#include "power.h"
#include "audioplayer.h"
#include "wlan.h"

using namespace std;

class BLERemote {
    private:
        shared_ptr<UserConfig> userConfig;
        shared_ptr<Power> power;
        shared_ptr<AudioPlayer> audioPlayer;
        shared_ptr<WLAN> wlan;
        TickType_t lastCharacteristicsUpdate;
        BLEServer* bleServer;
        BLEService* bleService;
        BLECharacteristic* powerCharacteristic;
        void updatePowerCharacteristic();
        BLECharacteristic* playerCharacteristic;
        void updatePlayerCharacteristic();
        BLECharacteristic* networkCharacteristic;
        void updateNetworkCharacteristic();
    public:
        BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power, shared_ptr<AudioPlayer> audioPlayer, shared_ptr<WLAN> wlan);
        void initialize();
        void bleRemoteLoop();
        void updateWifiState(bool connected, int32_t ipV4);
        void shutdown();
};
