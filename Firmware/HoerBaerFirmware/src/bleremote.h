#pragma once

#include <NimBLEDevice.h>
#include <memory>
#include <Wire.h>
#include <set>
#include "userconfig.h"
#include "power.h"
#include "audioplayer.h"
#include "wlan.h"

using namespace std;

class BLERemote {

    friend class BLERemoteServerCallbacks;
    friend class BLERemoteControlCallbacks;
    friend class BLERemotePlayerCommandCallbacks;

    private:
        shared_ptr<UserConfig> userConfig;
        shared_ptr<Power> power;
        shared_ptr<AudioPlayer> audioPlayer;
        shared_ptr<WLAN> wlan;
        BLEServer* bleServer;
        BLEService* bleService;

        BLECharacteristic* powerCharacteristic;
        void updatePowerCharacteristic();

        BLECharacteristic* playerCharacteristic;
        void updatePlayerCharacteristic();
        int volumeSent;
        int playingInfoSerialSent;

        BLECharacteristic* networkCharacteristic;
        void updateNetworkCharacteristic();

        BLECharacteristic* controlCharacteristic;
        void onControlReceived(NimBLECharacteristic* pCharacteristic); // has to be public for callbacks
        
        BLECharacteristic* playerCmdCharacteristic;
        void onPlayerCommandReceived(NimBLECharacteristic* pCharacteristic);
        void processPlayerCommand(const uint8_t* data, size_t length);
        
        std::set<uint16_t> connectedClients; // has to be public for callbacks
        
    public:
        BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power, shared_ptr<AudioPlayer> audioPlayer, shared_ptr<WLAN> wlan);
        void initialize();
        void runWorkerTask();
        void shutdown();
};


class BLERemoteServerCallbacks : public NimBLEServerCallbacks {
    private:
        BLERemote* bleRemote;
    public:
        BLERemoteServerCallbacks(BLERemote* ble) : bleRemote(ble) {}
        void onConnect(NimBLEServer* pServer, NimBLEConnInfo& info) override;
        void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& info, int reason) override;
};
    
class BLERemoteControlCallbacks : public NimBLECharacteristicCallbacks {
    private:
        BLERemote* bleRemote;
    public:
        BLERemoteControlCallbacks(BLERemote* ble) : bleRemote(ble) {}        
        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
};

class BLERemotePlayerCommandCallbacks : public NimBLECharacteristicCallbacks {
    private:
        BLERemote* bleRemote;
    public:
        BLERemotePlayerCommandCallbacks(BLERemote* ble) : bleRemote(ble) {}        
        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
};