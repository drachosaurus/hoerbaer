#include "log.h"
#include "config.h"
#include "userconfig.h"

#define DEFAULT_WIFI_SSID "myssid"
#define DEFAULT_WIFI_PASSWORD "mypassword"

UserConfig::UserConfig(std::shared_ptr<SDCard> sdCard)
{
    this->sdCard = sdCard;

    this->wifiConfig = std::make_shared<WifiConfig>();
    this->wifiConfig->enabled = false;
    this->wifiConfig->ssid = DEFAULT_WIFI_SSID;
    this->wifiConfig->password = DEFAULT_WIFI_PASSWORD;

    this->hbiConfig = std::make_shared<HBIConfig>();
    this->hbiConfig->reverseNose = false;
    this->hbiConfig->releaseInsteadOfPress = false;
    this->hbiConfig->ioMapping[0] = { IO_MAPPING_TYPE_PLAY_FOLDER, "/PAW1" };
    this->hbiConfig->ioMapping[1] = { IO_MAPPING_TYPE_PLAY_FOLDER, "/PAW2" };
    this->hbiConfig->ioMapping[2] = { IO_MAPPING_TYPE_PLAY_FOLDER, "/PAW3" };
    this->hbiConfig->ioMapping[3] = { IO_MAPPING_TYPE_PLAY_FOLDER, "/PAW4" };
    this->hbiConfig->ioMapping[4] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[5] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[6] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[7] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[8] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[9] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[10] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[11] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[12] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[13] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[14] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[15] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[16] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[17] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[18] = { IO_MAPPING_TYPE_NONE, "" };
    this->hbiConfig->ioMapping[19] = { IO_MAPPING_TYPE_CONTROL_STOP, "" };
    this->hbiConfig->ioMapping[20] = { IO_MAPPING_TYPE_CONTROL_PLAY, "" };
    this->hbiConfig->ioMapping[21] = { IO_MAPPING_TYPE_CONTROL_PAUSE, "" };
    this->hbiConfig->ioMapping[22] = { IO_MAPPING_TYPE_CONTROL_NEXT, "" };
    this->hbiConfig->ioMapping[23] = { IO_MAPPING_TYPE_CONTROL_PREV, "" };
}

void UserConfig::initializeFromSdCard()
{
    if(!this->sdCard->cardPresent())
    {
        Log::println("USRCFG", "SD card not present. Keeping defaults.");
        return;
    }

    try 
    {
        // Initialize WIFI settings
        StaticJsonDocument<192> wifiJson; // size calculated with https://arduinojson.org/v6/assistant
        if(this->sdCard->fileExists(SDCARD_FILE_WIFI_CONFIG)) {
            this->sdCard->readParseJsonFile(SDCARD_FILE_WIFI_CONFIG, wifiJson);
            this->wifiConfig->enabled = wifiJson["enabled"];
            this->wifiConfig->ssid = wifiJson["ssid"].as<std::string>();
            this->wifiConfig->password = wifiJson["password"].as<std::string>();
            Log::println("USRCFG", "Loaded WIFI config: enabled: %d, ssid: %s", this->wifiConfig->enabled, this->wifiConfig->ssid.c_str());
        }
        else {
            wifiJson["enabled"] = this->wifiConfig->enabled;
            wifiJson["ssid"] = this->wifiConfig->ssid;
            wifiJson["password"] = this->wifiConfig->password;
            this->sdCard->writeJsonFile(SDCARD_FILE_WIFI_CONFIG, wifiJson);
            Log::println("USRCFG", "Written default WIFI config.");
        }

        // Initialize IO settings
        StaticJsonDocument<4096> hbiConfigJson; // size calculated with https://arduinojson.org/v6/assistant
        if(this->sdCard->fileExists(SDCARD_FILE_HBI_CONFIG)) {
            this->sdCard->readParseJsonFile(SDCARD_FILE_HBI_CONFIG, hbiConfigJson);
            this->hbiConfig->reverseNose = hbiConfigJson["reverseNose"];
            this->hbiConfig->releaseInsteadOfPress = hbiConfigJson["releaseInsteadOfPress"] | false;
            Log::println("USRCFG", "Loaded HBI config:");
            Log::println("USRCFG", "- reverseNose: %s", this->hbiConfig->reverseNose ? "true" : "false");
            Log::println("USRCFG", "- releaseInsteadOfPress: %s", this->hbiConfig->releaseInsteadOfPress ? "true" : "false");
            for (int i = 0; i < 24; i++)
            {
                this->hbiConfig->ioMapping[i].type = hbiConfigJson["ioMapping"][i]["type"];
                this->hbiConfig->ioMapping[i].value = hbiConfigJson["ioMapping"][i]["value"].as<std::string>();
                Log::println("USRCFG", "- IO%d: 0x%02X %s", i, this->hbiConfig->ioMapping[i].type, this->hbiConfig->ioMapping[i].value.c_str());
            }
        }
        else {
            hbiConfigJson["reverseNose"] = this->hbiConfig->reverseNose;
            hbiConfigJson["releaseInsteadOfPress"] = this->hbiConfig->releaseInsteadOfPress;
            auto ioMapping = hbiConfigJson.createNestedArray("ioMapping");
            for(int i=0; i<24; i++) 
            {
                auto item = ioMapping.createNestedObject();
                item["type"] = this->hbiConfig->ioMapping[i].type;
                item["value"] = this->hbiConfig->ioMapping[i].value;
            }
            this->sdCard->writeJsonFile(SDCARD_FILE_HBI_CONFIG, hbiConfigJson);
            Log::println("USRCFG", "Written default IO config.");
        }
    }
    catch (const std::exception& e)
    {
        Log::println("USRCFG", "Unable to load " SDCARD_FILE_WIFI_CONFIG " - %s", e.what());
        return;
    }
    catch (...)
    {
        Log::println("USRCFG", "Unable to load " SDCARD_FILE_WIFI_CONFIG " - Unknown error");
        return;
    }
}

std::shared_ptr<WifiConfig> UserConfig::getWifiConfig()
{
    return this->wifiConfig;
}

std::shared_ptr<HBIConfig> UserConfig::getHBIConfig()
{
    return this->hbiConfig;
}
