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

shared_ptr<SDCard> sdCard;
shared_ptr<AudioPlayer> audioPlayer;
shared_ptr<UserConfig> userConfig;
unique_ptr<Power> power;
unique_ptr<HBI> hbi;

void shutdown();

void setup()
{
  i2c = make_shared<TwoWire>(0);
  i2cSema = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSema);

  // First (has to be first!), disable 3V3 ~PSAVE
  power = make_unique<Power>(i2c, i2cSema);
  power->DisableVCCPowerSave();

  i2c->begin(GPIO_I2C_SDA, GPIO_I2C_SCL, 100000);

  auto wakeupReason = esp_sleep_get_wakeup_cause();

  Log::init();
  Log::println("MAIN", "Hello Bear! Main runs on core: %d and woke up because %d", xPortGetCoreID(), wakeupReason);

  // switch (wakeup_reason) {
  //   case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
  //   case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
  //   case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
  //   case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
  //   case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
  //   default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  // }

  sdCard = make_shared<SDCard>();
  userConfig = make_shared<UserConfig>(sdCard);
  userConfig->initializeFromSdCard();

  // pulls NPDN down
  audioPlayer = make_shared<AudioPlayer>(i2c, i2cSema, userConfig, sdCard);

  power->InitializeChargerAndGauge();
  power->CheckBatteryVoltage();
  power->EnableAudioVoltage();

  // enables power
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

  hbi = make_unique<HBI>(i2c, i2cSema, userConfig->getHBIConfig(), audioPlayer, shutdown);
  hbi->start();

  Log::println("MAIN", "Baer initialized, ready to play!");
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

void shutdown() 
{
  Log::println("MAIN", "Shutting down...");
  // power->Shutdown();
  // delay(1000);
  // esp_deep_sleep_start();

  audioPlayer->stop();
  audioPlayer.reset();

  hbi->shutOffAllLeds();

  Log::println("MAIN", "Wait until encoder button is released!");
  hbi->waitUntilEncoderButtonReleased();
  hbi.reset();

  Log::println("MAIN", "Sleep well, bear!");

  power->DisableAudioVoltage();
  power->EnableVCCPowerSave();

  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(GPIO_HBI_ENCODER_BTN), 0);  //1 = High, 0 = Low
  esp_deep_sleep_start();
}

// #endif /* ARDUINO_USB_MODE */
