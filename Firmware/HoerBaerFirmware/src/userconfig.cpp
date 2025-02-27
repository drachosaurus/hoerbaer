#include "log.h"
#include "config.h"
#include "userconfig.h"

#define DEFAULT_WIFI_SSID "myssid"
#define DEFAULT_WIFI_PASSWORD "mypassword"

DynamicJsonDocument jsonBuffer(2048); // size calculated with https://arduinojson.org/v6/assistant

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
    this->hbiConfig->ioMapping[0] = IO_MAPPING_TYPE_PLAY_SLOT;
    this->hbiConfig->ioMapping[1] = IO_MAPPING_TYPE_PLAY_SLOT;
    this->hbiConfig->ioMapping[2] = IO_MAPPING_TYPE_PLAY_SLOT;
    this->hbiConfig->ioMapping[3] = IO_MAPPING_TYPE_PLAY_SLOT;
    this->hbiConfig->ioMapping[4] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[5] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[6] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[7] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[8] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[9] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[10] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[11] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[12] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[13] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[14] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[15] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[16] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[17] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[18] = IO_MAPPING_TYPE_NONE;
    this->hbiConfig->ioMapping[19] = IO_MAPPING_TYPE_CONTROL_STOP;
    this->hbiConfig->ioMapping[20] = IO_MAPPING_TYPE_CONTROL_PLAY;
    this->hbiConfig->ioMapping[21] = IO_MAPPING_TYPE_CONTROL_PAUSE;
    this->hbiConfig->ioMapping[22] = IO_MAPPING_TYPE_CONTROL_NEXT;
    this->hbiConfig->ioMapping[23] = IO_MAPPING_TYPE_CONTROL_PREV;

    this->audioConfig = std::make_shared<AudioConfig>();
    this->audioConfig->initalVolume = 130;
    this->audioConfig->minVolume = 0;
    this->audioConfig->maxVolume = 255;
    this->audioConfig->volumeEncoderStep = 5;

    this->slotDirectories = std::make_shared<vector<string>>();
    this->slotDirectories->push_back("/PAW01");
    this->slotDirectories->push_back("/PAW02");
    this->slotDirectories->push_back("/PAW03");
    this->slotDirectories->push_back("/PAW04");
}

void UserConfig::initializeFromSdCard()
{
    if(!this->sdCard->cardPresent())
    {
        Log::println("USRCFG", "SD card not present. Keeping defaults.");
        return;
    }

    this->initializeWifi();
    this->initializeHBI();
    this->initializeAudio();
    this->initializeSlots();
}

void UserConfig::initializeWifi() 
{
    try 
    {
        jsonBuffer.clear();
        if(this->sdCard->fileExists(SDCARD_FILE_WIFI_CONFIG)) 
        {
            this->sdCard->readParseJsonFile(SDCARD_FILE_WIFI_CONFIG, jsonBuffer);
            this->wifiConfig->enabled = jsonBuffer["enabled"];
            this->wifiConfig->ssid = jsonBuffer["ssid"].as<std::string>();
            this->wifiConfig->password = jsonBuffer["password"].as<std::string>();
            Log::println("USRCFG", "Loaded WIFI config: enabled: %d, ssid: %s", this->wifiConfig->enabled, this->wifiConfig->ssid.c_str());
        }
        else 
        {
            jsonBuffer["enabled"] = this->wifiConfig->enabled;
            jsonBuffer["ssid"] = this->wifiConfig->ssid;
            jsonBuffer["password"] = this->wifiConfig->password;
            this->sdCard->writeJsonFile(SDCARD_FILE_WIFI_CONFIG, jsonBuffer);
            Log::println("USRCFG", "Written default WIFI config.");
        }
    }
    catch (const std::exception& e)
    {
        Log::println("USRCFG", "Unable to initialize WIFI config - %s", e.what());
        return;
    }
    catch (...)
    {
        Log::println("USRCFG", "Unable to initialize WIFI config - Unknown error");
        return;
    }
}

void UserConfig::initializeHBI() 
{
    try 
    {
        jsonBuffer.clear();
        if(this->sdCard->fileExists(SDCARD_FILE_HBI_CONFIG)) 
        {
            this->sdCard->readParseJsonFile(SDCARD_FILE_HBI_CONFIG, jsonBuffer);
            this->hbiConfig->reverseNose = jsonBuffer["reverseNose"];
            this->hbiConfig->releaseInsteadOfPress = jsonBuffer["releaseInsteadOfPress"] | false;
            Log::println("USRCFG", "Loaded HBI config:");
            Log::println("USRCFG", "- reverseNose: %s", this->hbiConfig->reverseNose ? "true" : "false");
            Log::println("USRCFG", "- releaseInsteadOfPress: %s", this->hbiConfig->releaseInsteadOfPress ? "true" : "false");
            for (int i = 0; i < 24; i++)
            {
                this->hbiConfig->ioMapping[i] = jsonBuffer["ioMapping"][i];
                Log::println("USRCFG", "- IO%d: 0x%02X", i, this->hbiConfig->ioMapping[i]);
            }
        }
        else 
        {
            jsonBuffer["reverseNose"] = this->hbiConfig->reverseNose;
            jsonBuffer["releaseInsteadOfPress"] = this->hbiConfig->releaseInsteadOfPress;
            auto ioMapping = jsonBuffer.createNestedArray("ioMapping");
            for(int i=0; i<24; i++) 
                ioMapping.add(this->hbiConfig->ioMapping[i]);
            this->sdCard->writeJsonFile(SDCARD_FILE_HBI_CONFIG, jsonBuffer);
            Log::println("USRCFG", "Written default IO config.");
        }
    }
    catch (const std::exception& e)
    {
        Log::println("USRCFG", "Unable to initialize HBI config - %s", e.what());
        return;
    }
    catch (...)
    {
        Log::println("USRCFG", "Unable to initialize HBI config - Unknown error");
        return;
    }
}

void UserConfig::initializeAudio() 
{
    try 
    {
        jsonBuffer.clear();
        if(this->sdCard->fileExists(SDCARD_FILE_AUDIO_CONFIG)) 
        {
            this->sdCard->readParseJsonFile(SDCARD_FILE_AUDIO_CONFIG, jsonBuffer);
            this->audioConfig->initalVolume = jsonBuffer["initalVolume"];
            this->audioConfig->minVolume = jsonBuffer["minVolume"];
            this->audioConfig->maxVolume = jsonBuffer["maxVolume"];
            this->audioConfig->volumeEncoderStep = jsonBuffer["volumeEncoderStep"];
            Log::println("USRCFG", "Loaded Audio config: initalVolume: %d, minVolume: %d, maxVolume: %d, volumeEncoderStep: %d", 
                this->audioConfig->initalVolume, this->audioConfig->minVolume, this->audioConfig->maxVolume, this->audioConfig->volumeEncoderStep);
        }
        else 
        {
            jsonBuffer["initalVolume"] = this->audioConfig->initalVolume;
            jsonBuffer["minVolume"] = this->audioConfig->minVolume;
            jsonBuffer["maxVolume"] = this->audioConfig->maxVolume;
            jsonBuffer["volumeEncoderStep"] = this->audioConfig->volumeEncoderStep;
            this->sdCard->writeJsonFile(SDCARD_FILE_AUDIO_CONFIG, jsonBuffer);
            Log::println("USRCFG", "Written default Audio config.");
        }
    }
    catch (const std::exception& e)
    {
        Log::println("USRCFG", "Unable to initialize AUDIO config - %s", e.what());
        return;
    }
    catch (...)
    {
        Log::println("USRCFG", "Unable to initialize AUDIO config - Unknown error");
        return;
    }
}

void UserConfig::initializeSlots() 
{
    try 
    {
        jsonBuffer.clear();
        if(this->sdCard->fileExists(SDCARD_FILE_SLOTS_CONFIG)) 
        {
            this->sdCard->readParseJsonFile(SDCARD_FILE_SLOTS_CONFIG, jsonBuffer);
            this->slotDirectories->clear();

            auto slotsJsonArray = jsonBuffer.as<JsonArray>();
            Log::println("USRCFG", "Loaded Slots config:");
            for (JsonVariant slot : slotsJsonArray) 
            {
                this->slotDirectories->push_back(slot.as<std::string>());
                Log::println("USRCFG", "- %s", slot.as<std::string>().c_str());
            }
        }
        else
        {
            for (auto slotDirectory : *this->slotDirectories)
                jsonBuffer.add(slotDirectory);
            this->sdCard->writeJsonFile(SDCARD_FILE_SLOTS_CONFIG, jsonBuffer);
            Log::println("USRCFG", "Written default slots config.");
        }
    }
    catch (const std::exception& e)
    {
        Log::println("USRCFG", "Unable to initialize SLOTS config - %s", e.what());
        return;
    }
    catch (...)
    {
        Log::println("USRCFG", "Unable to initialize SLOTS config - Unknown error");
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

std::shared_ptr<AudioConfig> UserConfig::getAudioConfig()
{
    return this->audioConfig;
}

std::shared_ptr<vector<string>> UserConfig::getSlotDirectories()
{
    return this->slotDirectories;
}