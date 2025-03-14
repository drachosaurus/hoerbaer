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
#include "usb_msc.h"

using namespace std;

shared_ptr<TwoWire> i2c;
SemaphoreHandle_t i2cSema;

shared_ptr<SDCard> sdCard;
shared_ptr<AudioPlayer> audioPlayer;
shared_ptr<UserConfig> userConfig;
unique_ptr<Power> power;
unique_ptr<HBI> hbi;
unique_ptr<USBStorage> usbMsc;

std::string wifiSsid;
std::string wifiPwd;

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.begin(wifiSsid.c_str(), wifiPwd.c_str());
}

void shutdown();

void setup()
{
  i2c = make_shared<TwoWire>(0);
  i2cSema = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSema);

  // First (has to be first!), disable 3V3 ~PSAVE
  power = make_unique<Power>(i2c, i2cSema);
  power->disableVCCPowerSave();

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

  power->initializeChargerAndGauge();
  if(power->checkBatteryShutdown()) {
    shutdown();
    return;
  }

  power->enableAudioVoltage();

  // enables power
  audioPlayer->initialize();

  hbi = make_unique<HBI>(i2c, i2cSema, userConfig->getHBIConfig(), audioPlayer, shutdown);
  hbi->start();

  // init USB MSC
  usbMsc = make_unique<USBStorage>(sdCard);
  usbMsc->initialize();

  WiFi.disconnect();
  auto wifi = userConfig->getWifiConfig();
  if (wifi->enabled)
  {
    wifiSsid = wifi->ssid;
    wifiPwd = wifi->password;
  
    WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  
    Log::println("MAIN", "WiFi connecting to SSID: %s", wifiSsid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid.c_str(), wifiPwd.c_str());
    Log::println("MAIN", "WiFi connected to SSID: %s, IP Address: %s", wifiSsid.c_str(), WiFi.localIP().toString().c_str());  
  }
  else
    Log::println("MAIN", "WiFi disabled");

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
  if(power->checkBatteryShutdownLoop())
  {
    shutdown();
    return;
  }
}

void shutdown() 
{
  Log::println("MAIN", "Shutting down...");

  audioPlayer->stop();
  audioPlayer.reset();

  hbi->shutOffAllLeds();

  Log::println("MAIN", "Wait until encoder button is released!");
  hbi->waitUntilEncoderButtonReleased();

  Log::println("MAIN", "Sleep well, bear!");

  power->disableAudioVoltage();
  power->enableVCCPowerSave();
  power->setGaugeToSleep();

  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(GPIO_HBI_ENCODER_BTN), 0);  //1 = High, 0 = Low
  esp_deep_sleep_start();
}
