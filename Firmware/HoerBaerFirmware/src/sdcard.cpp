#include <Arduino.h>
#include "log.h"
#include "SD.h"
#include "config.h"

#include "sdcard.h"

SPIClass sdSpi;

SDCard::SDCard()
{
    sdSpi.begin(GPIO_SD_SCK, GPIO_SD_DO, GPIO_SD_DI, -1);
    pinMode(GPIO_SD_DETECT, INPUT);
}

bool SDCard::cardPresent()
{
    return digitalRead(GPIO_SD_DETECT) == LOW;
}

void SDCard::mountOrThrow()
{
    if(!this->cardPresent())
        throw std::runtime_error("SD card not present");

    if(this->cardMounted)
        return;

    if (!SD.begin(GPIO_SD_CS, sdSpi))
        throw std::runtime_error("Failed to mount SD card");

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
        throw std::runtime_error("SD card not present (but should be according to the detect-pin)");

    if (cardType == CARD_MMC)
        Log::println("SDCARD", "Mounted SD card (type MMC)");
    else if (cardType == CARD_SD)
        Log::println("SDCARD", "Mounted SD card (type SD)");
    else if (cardType == CARD_SDHC)
        Log::println("SDCARD", "Mounted SD card (type SDHC)");
    else
        Log::println("SDCARD", "Mounted SD card (type UNKNOWN)");

    this->cardMounted = true;
}

void SDCard::listFiles()
{
    this->mountOrThrow();

    File root = SD.open("/");

    if (!root)
    {
        Log::println("SDCARD", "Failed to open directory!");
        return;
    }

    if (!root.isDirectory())
    {
        Log::println("SDCARD", "Failed to list: not a directory!");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
            Log::println("SDCARD", "- D: %s", file.name());
        else
            Log::println("SDCARD", "- F: %s - SIZE: %d", file.name(), file.size());

        file = root.openNextFile();
    }
}

bool SDCard::fileExists(const std::string filename) 
{
    this->mountOrThrow();
    return SD.exists(filename.c_str());
}

void SDCard::writeJsonFile(const std::string filename, JsonDocument& jsonDocument)
{
    this->mountOrThrow();

    File file = SD.open(filename.c_str(), FILE_WRITE);
    if (!file)
        throw std::runtime_error("Failed to create file");

    serializeJsonPretty(jsonDocument, file);

    Log::println("SDCARD", "JSON file created: %s", filename.c_str());
    file.close();
}

void SDCard::readParseJsonFile(const std::string filename, JsonDocument& targetJsonDocument)
{
    this->mountOrThrow();

    File file = SD.open(filename.c_str());
    if (!file)
        throw std::runtime_error("Failed to open file");

    DeserializationError error = deserializeJson(targetJsonDocument, file);

    file.close();

    if (error != DeserializationError::Ok)
        throw std::runtime_error(error.c_str());
}