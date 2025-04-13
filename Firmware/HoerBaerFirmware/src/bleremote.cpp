#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <pb_encode.h>
#include "log.h"
#include "config.h"
#include "bleremote.h"

#include "power_state_characteristic.pb.h"

uint8_t pbBuffer[128]; // check all "pb.h"

BLERemote::BLERemote(shared_ptr<UserConfig> userConfig, shared_ptr<Power> power) {
    this->userConfig = userConfig;
    this->power = power;
}

void BLERemote::initialize() {

    Log::println("BLERemote", "Initializing BLE remote");

    BLEDevice::init(userConfig->getName().c_str());

    bleServer = BLEDevice::createServer();
    bleService = bleServer->createService(BLE_SERVICE_UUID);
    
    powerCharacteristic = bleService->createCharacteristic(BLE_CHARACTERISTIC_POWER_UUID, BLECharacteristic::PROPERTY_READ);
    playerCharacteristic = bleService->createCharacteristic(BLE_CHARACTERISTIC_PLAYER_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  
    powerCharacteristic->setValue("{}");
    playerCharacteristic->setValue("{}");
    
    bleService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Log::println("BLERemote", "Characteristic defined! Now you can read it in your phone!");
}

void BLERemote::updateCharacteristics() {

    auto state = power->getState();

    PowerStateCharacteristic message = PowerStateCharacteristic_init_zero;
    message.voltage = state.voltage;
    message.percentage = state.percentage;
    message.charging = state.charging;

    pb_ostream_t stream = pb_ostream_from_buffer(pbBuffer, sizeof(pbBuffer));
    if (!pb_encode(&stream, PowerStateCharacteristic_fields, &message))
        Log::println("BLERemote", "Failed to encode power state! Send defaults.");

    powerCharacteristic->setValue(pbBuffer, stream.bytes_written);
}

void BLERemote::shutdown() {
    BLEDevice::deinit(true);
    bleServer = nullptr;
    bleService = nullptr;
    powerCharacteristic = nullptr;
    playerCharacteristic = nullptr;
    Log::println("BLERemote", "BLE shut down");
}
