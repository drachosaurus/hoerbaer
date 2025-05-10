#include <Arduino.h>
#include <pb_encode.h>
#include "log.h"
#include "config.h"
#include "bleremote.h"

#include "power_state_characteristic.pb.h"
#include "player_state_characteristic.pb.h"
#include "network_state_characteristic.pb.h"

#define BLE_MAX_CONNECTIONS 5 // Configure: how many simultaneous connections you allow
#define PB_BUFFER_SIZE 512 // Buffer size for protobuf encoding (in PSRAM)

uint8_t* pbBuffer = nullptr; // Will be dynamically allocated with ps_malloc

void BLERemoteServerCallbacks::onConnect(NimBLEServer* pServer, NimBLEConnInfo& info) {
    bleRemote->connectedClients.insert(info.getConnHandle());
    Log::println("BLE", "Client connected, handle=%d, total=%d\n", info.getConnHandle(), bleRemote->connectedClients.size());
}

void BLERemoteServerCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& info, int reason) {
    bleRemote->connectedClients.erase(info.getConnHandle());
    Log::println("BLE", "Client disconnected, handle=%d, remaining=%d\n", info.getConnHandle(), bleRemote->connectedClients.size());
    
    // Restart advertising if no clients
    if (bleRemote->connectedClients.empty()) {
        Log::println("BLE", "No clients left, restart advertising...");
        pServer->getAdvertising()->start();
    }
}

void BLERemoteControlCallbacks::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
    if (bleRemote) {
        bleRemote->onControlReceived(pCharacteristic);
    }
}

BLERemote::BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power, shared_ptr<AudioPlayer> audioPlayer, shared_ptr<WLAN> wlan) {
    this->userConfig = userConfig;
    this->power = power;
    this->audioPlayer = audioPlayer;
    this->wlan = wlan;
}

void BLEWorkerTask(void* param) 
{
    BLERemote* ble = static_cast<BLERemote*>(param);
    ble->runWorkerTask();
}

void BLERemote::initialize() {

    Log::println("BLE", "Initializing BLE remote (NimBLE, Multi-client)");

    Log::logCurrentHeap("Before NimBLEDevice::init");

    NimBLEDevice::init(userConfig->getName().c_str());
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // maximum transmit power
    // NimBLEDevice::setMaxConnections(BLE_MAX_CONNECTIONS);
    NimBLEDevice::setMTU(128); // Optional: reduce memory usage
    NimBLEDevice::setSecurityAuth(false, false, true);

    Log::logCurrentHeap("After NimBLEDevice::init");

    bleServer = NimBLEDevice::createServer();
    bleServer->setCallbacks(new BLERemoteServerCallbacks(this));

    bleService = bleServer->createService(BLE_SERVICE_UUID);

    powerCharacteristic = bleService->createCharacteristic(
        BLE_CHARACTERISTIC_POWER_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    // powerCharacteristic->addDescriptor(new NimBLE2902());

    playerCharacteristic = bleService->createCharacteristic(
        BLE_CHARACTERISTIC_PLAYER_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    // playerCharacteristic->addDescriptor(new NimBLE2902());

    networkCharacteristic = bleService->createCharacteristic(
        BLE_CHARACTERISTIC_NETWORK_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    // networkCharacteristic->addDescriptor(new NimBLE2902());

    controlCharacteristic = bleService->createCharacteristic(
        BLE_CHARACTERISTIC_CONTROL_UUID,
        NIMBLE_PROPERTY::WRITE
    );

    controlCharacteristic->setCallbacks(new BLERemoteControlCallbacks(this));


    bleService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->start();

    Log::println("BLE", "BLE advertising started");

    // Allocate buffer in PSRAM
    pbBuffer = (uint8_t*) ps_malloc(PB_BUFFER_SIZE);
    if (!pbBuffer) {
        Log::println("BLE", "FATAL: Failed to allocate pbBuffer in PSRAM!");
        abort();
    }

    xTaskCreate(
        BLEWorkerTask, "ble_worker",
        TASK_STACK_SIZE_HBI_WORKER_WORDS,
        this,
        TASK_PRIO_HBI_WORKER,
        NULL
    );
}

void BLERemote::runWorkerTask() 
{
    while (true) 
    {
        if (!connectedClients.empty()) {
            updatePowerCharacteristic();
            updatePlayerCharacteristic();
            updateNetworkCharacteristic();
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void BLERemote::shutdown() {
    NimBLEDevice::deinit(true);
    bleServer = nullptr;
    bleService = nullptr;
    powerCharacteristic = nullptr;
    playerCharacteristic = nullptr;
    networkCharacteristic = nullptr;
    connectedClients.clear();
    
    if (pbBuffer) {
        free(pbBuffer);
        pbBuffer = nullptr;
    }

    Log::println("BLE", "BLE shut down");
}

void BLERemote::updatePowerCharacteristic() {
    auto powerState = power->getState();

    PowerStateCharacteristic powerMessage = PowerStateCharacteristic_init_zero;
    powerMessage.batteryPresent = userConfig->getBatteryPresent();
    powerMessage.batteryVoltage = powerState.voltage;
    powerMessage.batteryPercentage = powerState.percentage;
    powerMessage.charging = powerState.charging;

    pb_ostream_t powerStream = pb_ostream_from_buffer(pbBuffer, PB_BUFFER_SIZE);
    if (!pb_encode(&powerStream, PowerStateCharacteristic_fields, &powerMessage)) {
        Log::println("BLE", "Failed to encode power state!");
        return;
    }

    powerCharacteristic->setValue(pbBuffer, powerStream.bytes_written);
    powerCharacteristic->notify(); // Notify all connections
    delay(3);
}

void BLERemote::updatePlayerCharacteristic() {
    auto playingInfo = this->audioPlayer->getPlayingInfo();
    auto volume = this->audioPlayer->getCurrentVolume();

    if (playingInfo == nullptr && this->playingInfoSerialSent == 0)
        return;

    if(playingInfo != nullptr && playingInfo->serial == this->playingInfoSerialSent && volume == this->volumeSent)
        return; // No update needed

    this->playingInfoSerialSent = playingInfo != nullptr ? playingInfo->serial : 0;
    this->volumeSent = volume;

    PlayerStateCharacteristic playerMessage = PlayerStateCharacteristic_init_zero;
    playerMessage.volume = volume;
    playerMessage.maxVolume = this->audioPlayer->getMaxVolume();
    
    if (playingInfo != nullptr) {
        playerMessage.state = playingInfo->pausedAtPosition > 0 ? PlayerState_PLAYER_PAUSED : PlayerState_PLAYER_PLAYING;
        playerMessage.slotActive = playingInfo->slot;
        playerMessage.fileIndex = playingInfo->index;
        playerMessage.fileCount = playingInfo->total;
        playerMessage.currentTime = playingInfo->currentTime;
        playerMessage.duration = playingInfo->duration;
    } 
    else 
        playerMessage.state = PlayerState_PLAYER_STOPPED;

    pb_ostream_t playerStream = pb_ostream_from_buffer(pbBuffer, PB_BUFFER_SIZE);
    if (!pb_encode(&playerStream, PlayerStateCharacteristic_fields, &playerMessage)) {
        Log::println("BLE", "Failed to encode player state!");
        return;
    }

    playerCharacteristic->setValue(pbBuffer, playerStream.bytes_written);
    playerCharacteristic->notify();

    delay(3);
}

void BLERemote::updateNetworkCharacteristic() {

    NetworkStateCharacteristic networkMessage = NetworkStateCharacteristic_init_zero;

    if (this->wlan->getEnabled()) {
        networkMessage.enabled = true;
        networkMessage.connected = wlan->getConnected();
        networkMessage.ipV4Address = wlan->getIPV4();
        networkMessage.rssi = wlan->getRSSI();
    } else
        networkMessage.enabled = false;

    pb_ostream_t networkStream = pb_ostream_from_buffer(pbBuffer, PB_BUFFER_SIZE);
    if (!pb_encode(&networkStream, NetworkStateCharacteristic_fields, &networkMessage)) {
        Log::println("BLE", "Failed to encode network state!");
        return;
    }

    networkCharacteristic->setValue(pbBuffer, networkStream.bytes_written);
    networkCharacteristic->notify();
    delay(3);
}

void BLERemote::onControlReceived(NimBLECharacteristic* pCharacteristic) {
}

