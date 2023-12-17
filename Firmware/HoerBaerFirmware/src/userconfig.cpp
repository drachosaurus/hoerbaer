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

    this->ioConfig = std::make_shared<IOConfig>();
    this->ioConfig->reverseNose = false;
    this->ioConfig->ioMapping[0] = { IO_MAPPING_TYPE_PLAY_FOLDER, "PAW1" };
    this->ioConfig->ioMapping[1] = { IO_MAPPING_TYPE_PLAY_FOLDER, "PAW2" };
    this->ioConfig->ioMapping[2] = { IO_MAPPING_TYPE_PLAY_FOLDER, "PAW3" };
    this->ioConfig->ioMapping[3] = { IO_MAPPING_TYPE_PLAY_FOLDER, "PAW4" };
    this->ioConfig->ioMapping[4] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[5] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[6] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[7] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[8] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[9] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[10] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[11] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[12] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[13] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[14] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[15] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[16] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[17] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[18] = { IO_MAPPING_TYPE_NONE, "" };
    this->ioConfig->ioMapping[19] = { IO_MAPPING_TYPE_CONTROL_STOP, "" };
    this->ioConfig->ioMapping[20] = { IO_MAPPING_TYPE_CONTROL_PLAY, "" };
    this->ioConfig->ioMapping[21] = { IO_MAPPING_TYPE_CONTROL_PAUSE, "" };
    this->ioConfig->ioMapping[22] = { IO_MAPPING_TYPE_CONTROL_NEXT, "" };
    this->ioConfig->ioMapping[23] = { IO_MAPPING_TYPE_CONTROL_PREV, "" };
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
        StaticJsonDocument<4096> ioConfigJson; // size calculated with https://arduinojson.org/v6/assistant
        if(this->sdCard->fileExists(SDCARD_FILE_IO_CONFIG)) {
            this->sdCard->readParseJsonFile(SDCARD_FILE_IO_CONFIG, ioConfigJson);
            this->ioConfig->reverseNose = ioConfigJson["reverseNose"];
            Log::println("USRCFG", "Loaded IO config:");
            for (int i = 0; i < 24; i++)
            {
                this->ioConfig->ioMapping[i].type = ioConfigJson["ioMapping"][i]["type"];
                this->ioConfig->ioMapping[i].value = ioConfigJson["ioMapping"][i]["value"].as<std::string>();
                Log::println("USRCFG", "- %d: 0x%02X %s", i, this->ioConfig->ioMapping[i].type, this->ioConfig->ioMapping[i].value.c_str());
            }
        }
        else {
            ioConfigJson["reverseNose"] = this->ioConfig->reverseNose;
            auto ioMapping = ioConfigJson.createNestedArray("ioMapping");
            for(int i=0; i<24; i++) 
            {
                auto item = ioMapping.createNestedObject();
                item["type"] = this->ioConfig->ioMapping[i].type;
                item["value"] = this->ioConfig->ioMapping[i].value;
            }
            this->sdCard->writeJsonFile(SDCARD_FILE_IO_CONFIG, ioConfigJson);
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
