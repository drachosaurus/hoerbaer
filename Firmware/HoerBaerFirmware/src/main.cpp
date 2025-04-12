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
#include "bleremote.h"

using namespace std;

shared_ptr<TwoWire> i2c;
SemaphoreHandle_t i2cSema;

shared_ptr<SDCard> sdCard;
shared_ptr<AudioPlayer> audioPlayer;
shared_ptr<UserConfig> userConfig;
unique_ptr<Power> power;
unique_ptr<HBI> hbi;
unique_ptr<BLERemote> bleRemote;
unique_ptr<USBStorage> usbMsc;

std::string wifiSsid;
std::string wifiPwd;
std::string currentLogFileName;

bool usbStorageMode = false;

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Log::println("WiFi", "Connected to AccessPoint.");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Log::println("WiFi", "IP Address: %s", WiFi.localIP().toString().c_str());
  configTime(0, 0, "pool.ntp.org"); // First connect to NTP server, with 0 TZ offset
  auto tz = userConfig->getTimezone();
  setenv("TZ", tz.c_str(), 1);         // set timezone
  tzset();
  Log::println("WiFi", "NTP time set: %s", tz.c_str());
  struct tm timeInfo;
  if (getLocalTime(&timeInfo)) {
    char timeStringBuff[22]; 
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d_%H%M%S.log", &timeInfo);
    Log::println("WiFi", "Set timestamp for logfile: %s", timeStringBuff);
    currentLogFileName = std::string(timeStringBuff);
  }
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
  Log::println("MAIN", "Hello Bear! I woke up because %d", wakeupReason);

  if(wakeupReason != ESP_SLEEP_WAKEUP_EXT0)
  {
    Log::println("MAIN", "Wakeup was not caused from button interrupt, shutting down again.");
    shutdown();
    return;
  }

  Log::println("MAIN", "Startup! \n"
        "\t- Main runs on core: %d \n" 
        "\t- PSRAM is %s\n"
        "\t- PINOUT_PCB_REV: %d \n",
        xPortGetCoreID(), 
        psramFound() ? "available" : "not available",
        PINOUT_PCB_REV);

  sdCard = make_shared<SDCard>();
  userConfig = make_shared<UserConfig>(sdCard);
  userConfig->initializeFromSdCard();

  // keeps 12V supply off (NPDN down) - has to be before HBI
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

    audioPlayer->initialize();

    bleRemote = make_unique<BLERemote>(userConfig);
    bleRemote->initialize();

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

  if (!usbStorageMode) {
    // Normal mode: audio loop and check battery
    audioPlayer->loop();
    bleRemote->updateCharacteristics();
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

  if(bleRemote != nullptr) {
    bleRemote->shutdown();
    bleRemote.reset();
  }

  if(audioPlayer != nullptr) {
    audioPlayer->stop();
    audioPlayer.reset();
  }

  if(hbi != nullptr) {
    hbi->shutOffAllLeds();
    
    Log::println("MAIN", "Wait until encoder button is released!");
    hbi->waitUntilEncoderButtonReleased();
  }

  Log::println("MAIN", "Sleep well, bear!");

  power->disableAudioVoltage();
  power->setGaugeToSleep();
  power->enableVCCPowerSave();

  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(GPIO_HBI_ENCODER_BTN), 0); // 1 = High, 0 = Low
  esp_deep_sleep_start();
}
