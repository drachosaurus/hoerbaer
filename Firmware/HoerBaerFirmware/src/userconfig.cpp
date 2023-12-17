#include <ArduinoJson.h>
#include "log.h"
#include "config.h"
#include "userconfig.h"

#define DEFAULT_WIFI_SSID "myssid"
#define DEFAULT_WIFI_PASSWORD "mypassword"
#define DEFAULT_WIFI_CONFIG "{ \"enabled\": false, \"ssid\": \"" DEFAULT_WIFI_SSID "\", \"password\": \"" DEFAULT_WIFI_PASSWORD "\" }"

UserConfig::UserConfig(std::shared_ptr<SDCard> sdCard)
{
    this->sdCard = sdCard;
    this->wifiEnabled = false;
    this->wifiSSID = DEFAULT_WIFI_SSID;
    this->wifiPassword = DEFAULT_WIFI_PASSWORD;
}

void UserConfig::initializeFromSdCard()
{
    if(!this->sdCard->cardPresent())
    {
        Log::println("USERCONFIG", "SD card not present. Keeping defaults.");
        return;
    }

    try 
    {
        this->sdCard->checkCreateFile(SDCARD_FILE_WIFI_CONFIG, DEFAULT_WIFI_CONFIG);
    }
    catch (const std::exception& e)
    {
        Log::println("USERCONFIG", "Unable to load wifi config: %s", e.what());
        return;
    }
    catch (...)
    {
        Log::println("USERCONFIG", "Unable to load wifi config.");
        return;
    }

    File file = SD.open(SDCARD_FILE_WIFI_CONFIG);

    // size calculated with https://arduinojson.org/v6/assistant
    StaticJsonDocument<192> doc;
    DeserializationError error = deserializeJson(doc, file);

    if(error != DeserializationError::Ok)
    {
        Log::println("USERCONFIG", "Unable to deserialize user config: %s", error.c_str());
        return;
    }

    this->wifiEnabled = doc["enabled"];
    this->wifiSSID = doc["ssid"].as<std::string>();
    this->wifiPassword = doc["password"].as<std::string>();
    Log::println("USERCONFIG", "Loaded user config: enabled: %d, ssid: %s", this->wifiEnabled, this->wifiSSID.c_str());
}

bool UserConfig::getWifiEnabled()
{
    return this->wifiEnabled;
}

std::string UserConfig::getWifiSSID()
{
    return this->wifiSSID;
}

std::string UserConfig::getWifiPassword()
{
    return this->wifiPassword;
}
