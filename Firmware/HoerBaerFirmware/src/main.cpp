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
#include "wlan.h"
#include "sdcard.h"
#include "userconfig.h"
#include "bleremote.h"
#include "webserver.h"
#include "usb_msc.h"

using namespace std;

shared_ptr<TwoWire> i2c;
SemaphoreHandle_t i2cSema;

shared_ptr<SDCard> sdCard;
shared_ptr<AudioPlayer> audioPlayer;
shared_ptr<UserConfig> userConfig;
shared_ptr<Power> power;
unique_ptr<HBI> hbi;
shared_ptr<WLAN> wlan;
unique_ptr<BLERemote> bleRemote;
shared_ptr<WebServer> webServer;
unique_ptr<USBStorage> usbMsc;

std::string wifiSsid;
std::string wifiPwd;
std::string currentLogFileName;

bool usbStorageMode = false;
bool shuttingDown = false;
TickType_t lastMemoryPrintout = 0;

void shutdown();

void setup() {

  try {

    shuttingDown = false;
    i2c = make_shared<TwoWire>(0);
    i2cSema = xSemaphoreCreateBinary();
    xSemaphoreGive(i2cSema);

    // First (has to be first!), disable 3V3 ~PSAVE
    power = make_shared<Power>(i2c, i2cSema);
    power->disableVCCPowerSave();

    i2c->begin(GPIO_I2C_SDA, GPIO_I2C_SCL, 100000);

    auto wakeupReason = esp_sleep_get_wakeup_cause();

    Log::init();
    Log::println("MAIN", "Hello Bear! I woke up because %d", wakeupReason);

    if (wakeupReason != ESP_SLEEP_WAKEUP_EXT0) {
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

    Log::logCurrentHeap("Start startup");

    sdCard = make_shared<SDCard>();
    userConfig = make_shared<UserConfig>(sdCard);
    userConfig->initializeFromSdCard();

    Log::logCurrentHeap("After user config");


    // keeps 12V supply off (NPDN down) - has to be before HBI
    audioPlayer = make_shared<AudioPlayer>(i2c, i2cSema, userConfig, sdCard);

    Log::logCurrentHeap("After audio player constructor");


    hbi = make_unique<HBI>(i2c, i2cSema, userConfig->getHBIConfig(), audioPlayer, shutdown);
    hbi->initialize();

    Log::logCurrentHeap("After HBI init");

    wlan = make_shared<WLAN>(userConfig);

    Log::logCurrentHeap("After WLAN init");

    power->initializeChargerAndGauge(userConfig->getBatteryPresent());
    if (power->checkBatteryShutdown()) {
      shutdown();
      return;
    }

    usbStorageMode = hbi->getAnyButtonPressed(); // any button pressed during boot will enable USB Storage mode

    if (!usbStorageMode) {

      power->enableAudioVoltage();

      audioPlayer->initialize();
      audioPlayer->populateAudioMetadata();

      Log::logCurrentHeap("After Audio init");

      bleRemote = make_unique<BLERemote>(userConfig, power, audioPlayer, wlan);
      bleRemote->initialize();

      Log::logCurrentHeap("After BLE init");

      hbi->setReadyToPlay(true);
      hbi->setActionButtonsEnabled(true);

      wlan->connectIfConfigured();

      if (wlan->getEnabled()) {
        Log::println("WLAN", "Starting WebServer");
        webServer = make_shared<WebServer>(audioPlayer);
        webServer->start();
      }

      Log::println("MAIN", "Baer initialized, ready to play!");
    }
    else {
      Log::println("MAIN", "Initialize USB Storage mode.");

      // init USB MSC
      usbMsc = make_unique<USBStorage>(sdCard);
      usbMsc->initialize();

      Log::println("MAIN", "Baer initialized in USB Mode, fill my stomache!");
    }
  }
  catch (const std::exception& e) {
    Log::println("MAIN", "Exception during setup: %s", e.what());
    ESP.restart();
    return;
  }
  catch (...) {
    Log::println("MAIN", "Unknown exception during setup");
    ESP.restart();
    return;
  }
}

void loop() {

  if (shuttingDown)
    return;

  if (!usbStorageMode) {
    // Normal mode: audio loop and check battery
    audioPlayer->loop();
    if (power->checkBatteryShutdownLoop())
    {
      shutdown();
      return;
    }

    if (lastMemoryPrintout == 0 || xTaskGetTickCount() - lastMemoryPrintout > pdMS_TO_TICKS(10000))
    {
      lastMemoryPrintout = xTaskGetTickCount();
#if ( PRINT_MEMORY_INFO == 1 )
      Log::printMemoryInfo();
#endif
#if ( PRINT_TASK_INFO == 1 )
      Log::printTaskInfo();
#endif
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
  shuttingDown = true;

  if (bleRemote != nullptr) {
    bleRemote->shutdown();
    bleRemote.reset();
  }

  if (audioPlayer != nullptr) {
    audioPlayer->stop();
    audioPlayer.reset();
  }

  if (hbi != nullptr) {
    hbi->setReadyToPlay(false);
    hbi->shutOffAllLeds();

    Log::println("MAIN", "Wait until encoder button is released!");
    hbi->waitUntilEncoderButtonReleased();
  }

  if (wlan != nullptr) {
    wlan->disconnect();
    wlan.reset();
  }

  Log::println("MAIN", "Sleep well, bear!");

  power->disableAudioVoltage();
  power->setGaugeToSleep();
  power->enableVCCPowerSave();

  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(GPIO_HBI_ENCODER_BTN), 0); // 1 = High, 0 = Low
  esp_deep_sleep_start();
}
