#include <WiFi.h>
#include "wlan.h"
#include "log.h"

WLAN::WLAN(std::shared_ptr<UserConfig> userConfig) {
  this->userConfig = userConfig;
}

void WLAN::connectIfConfigured() {
  this->disconnect();
  auto wifi = userConfig->getWifiConfig();
  if (wifi->enabled)
  {
    ssid = wifi->ssid;
    password = wifi->password;

    WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
      this->connected = true;
      Log::println("WiFi", "Connected to AccessPoint.");
    }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

    WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
      this->connected = true;
      this->ipV4 = WiFi.localIP();
      Log::println("WiFi", "IP Address: %s", WiFi.localIP().toString().c_str());
      configTime(0, 0, "pool.ntp.org");
      auto tz = userConfig->getTimezone();
      setenv("TZ", tz.c_str(), 1);
      tzset();
      Log::println("WiFi", "NTP time set: %s", tz.c_str());
    }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
      this->connected = false;
      Log::println("WiFi", "WiFi lost connection. Reason: %d. Trying to Reconnect...", info.wifi_sta_disconnected.reason);
      WiFi.begin(ssid.c_str(), password.c_str());
    }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    Log::println("WLAN", "WiFi connecting to SSID: %s", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setHostname(userConfig->getName().c_str());
  }
  else
    Log::println("WLAN", "WiFi disabled");
}

void WLAN::disconnect() {
  WiFi.disconnect();
}

bool WLAN::getEnabled() {
  return this->userConfig->getWifiConfig()->enabled;
}

bool WLAN::getConnected() {
  return this->connected;
}

std::string WLAN::getSSID() {
  return this->ssid;
}

std::string WLAN::getHostname() {
  return this->userConfig->getName();
}

int32_t WLAN::getIPV4() {
  return this->ipV4;
}

uint8_t WLAN::getRSSI() {
  return WiFi.RSSI();
}
