#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <memory>

#include "log.h"
#include "power.h"
#include "hbi.h"
#include "audioplayer.h"
#include "config.h"
#include "utils.h"
#include "sdcard.h"
#include "userconfig.h"

using namespace std;

shared_ptr<TwoWire> i2c;
SemaphoreHandle_t i2cSema;

shared_ptr<UserConfig> userConfig;
shared_ptr<SDCard> sdCard;

unique_ptr<Power> power;
unique_ptr<HBI> hbi;
unique_ptr<AudioPlayer> audioPlayer;

void setup()
{
  i2c = make_shared<TwoWire>(0);
  i2cSema = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSema);

  // First (has to be first!), disable 3V3 ~PSAVE
  power = make_unique<Power>(i2c, i2cSema);
  power->DisableVCCPowerSave();

  i2c->begin(GPIO_I2C_SDA, GPIO_I2C_SCL, 100000);

  Log::init();
  Log::println("MAIN", "Hello Bear! Main runs on core: %d", xPortGetCoreID());

  sdCard = make_shared<SDCard>();
  userConfig = make_shared<UserConfig>(sdCard);
  userConfig->initializeFromSdCard();

  power->EnableAudioVoltage();
  power->InitializeChargerAndGauge();
  power->CheckBatteryVoltage();

  audioPlayer = make_unique<AudioPlayer>(i2c, i2cSema);
  audioPlayer->initialize();

  WiFi.disconnect();
  auto wifi = userConfig->getWifiConfig();
  if (wifi->enabled)
  {
    Log::println("MAIN", "WiFi connecting to SSID: %s", wifi->ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi->ssid.c_str(), wifi->password.c_str());
    while (WiFi.status() != WL_CONNECTED)
      delay(500);
    Log::println("MAIN", "WiFi connected to SSID: %s", wifi->ssid.c_str());
  }
  else
    Log::println("MAIN", "WiFi disabled");

  hbi = make_unique<HBI>(i2c, i2cSema);
  hbi->start();

  // hbi->enableVegas();
  // audioPlayer->test();
}

void loop()
{
  // put your main code here, to run repeatedly
  // Serial.println(".");
  // hbi->enableVegas();
  // sleep(1);
  // hbi->disableVegas();
  audioPlayer->loop();
}
// #endif /* ARDUINO_USB_MODE */
