#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <pb_encode.h>
#include "log.h"
#include "config.h"
#include "bleremote.h"

#include "power_state_characteristic.pb.h"

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

BLERemote::BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power) {
    this->userConfig = userConfig;
    this->power = power;
}

void BLERemote::initialize() {

    Log::println("BLE", "Initializing BLE remote");

    BLEDevice::init(userConfig->getName().c_str());

    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new MyServerCallbacks());

    bleService = bleServer->createService(BLE_SERVICE_UUID);
    
    powerCharacteristic = bleService->createCharacteristic(BLE_CHARACTERISTIC_POWER_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    powerCharacteristic->addDescriptor(new BLE2902());

    playerCharacteristic = bleService->createCharacteristic(BLE_CHARACTERISTIC_PLAYER_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  
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
}

void BLERemote::bleRemoteLoop() {

    auto tickCount = xTaskGetTickCount();
    if(tickCount - lastCharacteristicsUpdate < pdMS_TO_TICKS(BLE_CHARACTERISTICS_UPDATE_INTERVAL_MILLIS))
      return;

    lastCharacteristicsUpdate = tickCount;

    // connected
    if (deviceConnected) {
        auto state = power->getState();
        PowerStateCharacteristic message = PowerStateCharacteristic_init_zero;
        message.batteryPresent = userConfig->getBatteryPresent();
        message.batteryVoltage = state.voltage;
        message.batteryPercentage = state.percentage;
        message.charging = state.charging;
    
        pb_ostream_t stream = pb_ostream_from_buffer(pbBuffer, sizeof(pbBuffer));
        if (!pb_encode(&stream, PowerStateCharacteristic_fields, &message))
            Log::println("BLE", "Failed to encode power state! Send defaults.");
    
        powerCharacteristic->setValue(pbBuffer, stream.bytes_written);
        powerCharacteristic->notify();
        delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
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
}

void BLERemote::shutdown() {
    BLEDevice::deinit(true);
    bleServer = nullptr;
    bleService = nullptr;
    powerCharacteristic = nullptr;
    playerCharacteristic = nullptr;
    Log::println("BLE", "BLE shut down");
}
