#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <pb_encode.h>
#include "log.h"
#include "config.h"
#include "bleremote.h"

#include "power_state_characteristic.pb.h"
#include "player_state_characteristic.pb.h"
#include "network_state_characteristic.pb.h"

uint8_t pbBuffer[128]; // check all "pb.h"
bool deviceConnected = false;
bool oldDeviceConnected = false;

static BLEDescriptor* delayDescriptor = new BLEDescriptor((uint16_t)0x2901);

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

BLERemote::BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power, shared_ptr<AudioPlayer> audioPlayer, shared_ptr<WLAN> wlan) {
    this->userConfig = userConfig;
    this->power = power;
    this->audioPlayer = audioPlayer;
    this->wlan = wlan;
}

void BLEWorkerTask(void * param) 
{
    BLERemote* ble = (static_cast<BLERemote*>(param));
    ble->runWorkerTask();
}

void BLERemote::initialize() {

    Log::println("BLE", "Initializing BLE remote");
    
    Log::logCurrentHeap("Before BLEDevice::init");
    BLEDevice::init(userConfig->getName().c_str());
    Log::logCurrentHeap("After BLEDevice::init");

    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new MyServerCallbacks());

    Log::logCurrentHeap("After BLEDevice::createServer");

    bleService = bleServer->createService(BLE_SERVICE_UUID);
    Log::logCurrentHeap("After bleServer->createService");

    powerCharacteristic = bleService->createCharacteristic(BLE_CHARACTERISTIC_POWER_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    powerCharacteristic->addDescriptor(new BLE2902());
    Log::logCurrentHeap("After createCharacteristic 1");

    playerCharacteristic = bleService->createCharacteristic(BLE_CHARACTERISTIC_PLAYER_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    playerCharacteristic->addDescriptor(new BLE2902());
    Log::logCurrentHeap("After createCharacteristic 2");

    networkCharacteristic = bleService->createCharacteristic(BLE_CHARACTERISTIC_NETWORK_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    networkCharacteristic->addDescriptor(new BLE2902());
    Log::logCurrentHeap("After createCharacteristic 3");

    // powerCharacteristic->setValue();
    // playerCharacteristic->setValue();
    
    bleService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Log::println("BLE", "Characteristic defined! Now you can read it in your phone!");

    xTaskCreate(BLEWorkerTask, "ble_worker", 
        TASK_STACK_SIZE_HBI_WORKER_WORDS, 
        this, 
        TASK_PRIO_HBI_WORKER,
        NULL);
}

void BLERemote::runWorkerTask() 
{
    while(1) 
    {
        // connected
        if (deviceConnected) {
            updatePowerCharacteristic();
            updatePlayerCharacteristic();
            updateNetworkCharacteristic();
        }
    
        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            vTaskDelay(pdMS_TO_TICKS(500));
            bleServer->startAdvertising(); // restart advertising
            Log::println("BLE", "Client disconnecting, start advertising...");
            oldDeviceConnected = deviceConnected;
        }
    
        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
            Log::println("BLE", "Client connecting...");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void BLERemote::shutdown() {
    BLEDevice::deinit(true);
    bleServer = nullptr;
    bleService = nullptr;
    powerCharacteristic = nullptr;
    playerCharacteristic = nullptr;
    Log::println("BLE", "BLE shut down");
}


void BLERemote::updatePowerCharacteristic() {
    auto powerState = power->getState();

    PowerStateCharacteristic powerMessage = PowerStateCharacteristic_init_zero;
    powerMessage.batteryPresent = userConfig->getBatteryPresent();
    powerMessage.batteryVoltage = powerState.voltage;
    powerMessage.batteryPercentage = powerState.percentage;
    powerMessage.charging = powerState.charging;

    pb_ostream_t powerStream = pb_ostream_from_buffer(pbBuffer, sizeof(pbBuffer));
    if (!pb_encode(&powerStream, PowerStateCharacteristic_fields, &powerMessage))
        Log::println("BLE", "Failed to encode power state! Send defaults.");

    powerCharacteristic->setValue(pbBuffer, powerStream.bytes_written);
    powerCharacteristic->notify();
    delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
}

void BLERemote::updatePlayerCharacteristic() {
    auto playingInfo = this->audioPlayer->getPlayingInfo();

    PlayerStateCharacteristic playerMessage = PlayerStateCharacteristic_init_zero;
    playerMessage.volume = this->audioPlayer->getCurrentVolume();
    playerMessage.maxVolume = this->audioPlayer->getMaxVolume();
    if(playingInfo != nullptr)
    {
        playerMessage.state = playingInfo->pausedAtPosition > 0 ? PlayerState_PLAYER_PAUSED : PlayerState_PLAYER_PLAYING;
        playerMessage.slotActive = playingInfo->slot;
        playerMessage.fileIndex = playingInfo->index;
        playerMessage.fileCount = playingInfo->total;
        playerMessage.currentTime = playingInfo->currentTime;
        playerMessage.duration = playingInfo->duration;
    }
    else
        playerMessage.state = PlayerState_PLAYER_STOPPED;

    pb_ostream_t playerStream = pb_ostream_from_buffer(pbBuffer, sizeof(pbBuffer));
    if (!pb_encode(&playerStream, PlayerStateCharacteristic_fields, &playerMessage))
        Log::println("BLE", "Failed to encode player state! Send defaults.");

    playerCharacteristic->setValue(pbBuffer, playerStream.bytes_written);
    playerCharacteristic->notify();
    delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
}

void BLERemote::updateNetworkCharacteristic() {
    
    NetworkStateCharacteristic networkMessage = NetworkStateCharacteristic_init_zero;
    std::string ssid = wlan->getSSID();

    if(this->wlan->getEnabled())
    {
        networkMessage.enabled = true;
        networkMessage.connected = wlan->getConnected();
        networkMessage.ipV4Address = wlan->getIPV4();
        networkMessage.rssi = wlan->getRSSI();
    }
    else
        networkMessage.enabled = false;

    pb_ostream_t networkStream = pb_ostream_from_buffer(pbBuffer, sizeof(pbBuffer));
    if (!pb_encode(&networkStream, NetworkStateCharacteristic_fields, &networkMessage))
        Log::println("BLE", "Failed to network state! Send defaults.");

    networkCharacteristic->setValue(pbBuffer, networkStream.bytes_written);
    networkCharacteristic->notify();
    delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
}