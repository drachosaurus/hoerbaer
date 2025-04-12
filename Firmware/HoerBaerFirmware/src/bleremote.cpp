#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "log.h"
#include "config.h"
#include "bleremote.h"

BLERemote::BLERemote(shared_ptr<UserConfig> userConfig) {
    this->userConfig = userConfig;
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

    Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void BLERemote::updateCharacteristics() {
}

void BLERemote::shutdown() {
    BLEDevice::deinit(true);
    bleServer = nullptr;
    bleService = nullptr;
    powerCharacteristic = nullptr;
    playerCharacteristic = nullptr;
    Log::println("BLERemote", "BLE shut down");
}
