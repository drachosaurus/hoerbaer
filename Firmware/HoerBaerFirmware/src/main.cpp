#include <Arduino.h>
#include <Wire.h>
#include <memory>

#include "log.h"
#include "power.h"
#include "hbi.h"
#include "config.h"

using namespace std;

shared_ptr<TwoWire> i2c;
SemaphoreHandle_t i2cSema;

unique_ptr<Power> power;
unique_ptr<HBI> hbi;

void setup() {

  i2c = make_shared<TwoWire>(0);
  i2cSema = xSemaphoreCreateBinary();

  // First (has to be first!), disable 3V3 ~PSAVE
  power = make_unique<Power>();
  power->DisableVCCPowerSave();

  i2c->begin(GPIO_I2C_SDA, GPIO_I2C_SCL, 100000);
  power->EnableAudioVoltage();

  Log::init();
  Log::println("Hello Bear! Main runs on core: %d", xPortGetCoreID());

  hbi = make_unique<HBI>(i2c, i2cSema);
  hbi->startInputListener();

  // Serial.setDebugOutput(true);
  // Serial.println("Start i2c...");
  // i2c.begin(GPIO_I2C_SDA, GPIO_I2C_SCL, 100000);


  // Serial.println("Scanning...");

  // byte error, address;
  // int nDevices = 0;
  // for(address = 1; address < 127; address++ ) 
  // {
  //   // The i2c_scanner uses the return value of
  //   // the Write.endTransmisstion to see if
  //   // a device did acknowledge to the address.
  //   i2c.beginTransmission(address);
  //   error = i2c.endTransmission();

  //   if (error == 0)
  //   {
  //     Serial.print("I2C device found at address 0x");
  //     if (address<16) 
  //       Serial.print("0");
  //     Serial.print(address, HEX);
  //     Serial.println("  !");

  //     nDevices++;
  //   }
  //   else if (error==4) 
  //   {
  //     Serial.print("Unknown error at address 0x");
  //     if (address<16) 
  //       Serial.print("0");
  //     Serial.println(address,HEX);
  //   }    
  // }
  // if (nDevices == 0)
  //   Serial.println("No I2C devices found\n");
  // else
  //   Serial.println("done\n");









  // USB.onEvent(usbEventCallback);
  // MSC_Update.onEvent(usbEventCallback);
  // MSC_Update.begin();
  // USBSerial.begin();
  // USB.begin();
}

void loop() {
  // put your main code here, to run repeatedly
  // Serial.println(".");
  sleep(1);
}
// #endif /* ARDUINO_USB_MODE */
