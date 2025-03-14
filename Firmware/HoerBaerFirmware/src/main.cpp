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

bool usbStorageMode = false;

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Log::println("WiFi", "Connected to AccessPoint.");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Log::println("WiFi", "IP Address: %s", WiFi.localIP().toString().c_str());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Log::println("WiFi", "WiFi lost connection. Reason: %d. Trying to Reconnect...", info.wifi_sta_disconnected.reason);
  WiFi.begin(wifiSsid.c_str(), wifiPwd.c_str());
}

void shutdown();

void setup() {
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

  // pulls NPDN down - has to be before HBI
  audioPlayer = make_shared<AudioPlayer>(i2c, i2cSema, userConfig, sdCard);

  hbi = make_unique<HBI>(i2c, i2cSema, userConfig->getHBIConfig(), audioPlayer, shutdown);
  hbi->initialize();

  power->initializeChargerAndGauge();
  if (power->checkBatteryShutdown()) {
    shutdown();
    return;
  }

  usbStorageMode = hbi->getAnyButtonPressed(); // any button pressed during boot will enable USB Storage mode

  if (!usbStorageMode) {

    power->enableAudioVoltage();

    // enables power
    audioPlayer->initialize();

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
    }
    else
      Log::println("MAIN", "WiFi disabled");
      
    Log::println("MAIN", "Baer initialized, ready to play!");
  }
  else
  {
    Log::println("MAIN", "Initialize USB Storage mode.");

    // init USB MSC
    usbMsc = make_unique<USBStorage>(sdCard);
    usbMsc->initialize();

    Log::println("MAIN", "Baer initialized in USB Mode, fill my stomache!");
  }
}

void loop() {
  if (!usbStorageMode)
  {
    // Normal mode: audio loop and check battery
    audioPlayer->loop();
    if (power->checkBatteryShutdownLoop())
    {
      shutdown();
      return;
    }
  }
  else {
    // USB Storage mode: do nothing, just blink HMI
    hbi->runVegasStep();
    delay(200);
  }
}

void shutdown() {
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

  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(GPIO_HBI_ENCODER_BTN), 0); // 1 = High, 0 = Low
  esp_deep_sleep_start();
}
