#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "log.h"
#include "bleremote.h"

#define SERVICE_UUID        "10fe50c2-7787-425f-9d74-187313ca41d4"
#define CHARACTERISTIC_UUID "bdb1d967-8a30-42fd-b035-0b65e15074ca"

BLERemote::BLERemote() {

}

void BLERemote::initialize() {

    Log::println("BLERemote", "Initializing BLE remote");

    BLEDevice::init("Long name works now");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    BLECharacteristic *pCharacteristic =
      pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  
    pCharacteristic->setValue("Hello World says Neil");
    pService->start();
    
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}
