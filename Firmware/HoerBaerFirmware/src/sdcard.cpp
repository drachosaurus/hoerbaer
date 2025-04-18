#include <Arduino.h>
#include "log.h"
#include "SD.h"
#include "SD_MMC.h"
#include "config.h"

#include "sdcard.h"

#ifdef SD_MODE_SDMMC
    #define SDLIB SD_MMC
#else
    #define SDLIB SD
    SPIClass sdSpi;
#endif

SDCard::SDCard()
{
    #ifdef SD_MODE_SDMMC
        SD_MMC.setPins(GPIO_SD_CLK, GPIO_SD_CMD, GPIO_SD_D0, GPIO_SD_D1, GPIO_SD_D2, GPIO_SD_D3);
    #else
        sdSpi.begin(GPIO_SD_SCK, GPIO_SD_DO, GPIO_SD_DI, -1);
    #endif

    pinMode(GPIO_SD_DETECT, INPUT);
}

FSTYPE& SDCard::getFs()
{
    return SDLIB;
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

    Log::println("SDCARD", "Mounting SD card...");

    uint8_t cardType = CARD_NONE;

#ifdef SD_MODE_SDMMC
    if (!SD_MMC.begin("/sdcard", false, false, SDMMC_FREQ))
        throw std::runtime_error("Failed to mount SD card");
#else
    if (!SD.begin(GPIO_SD_CS, sdSpi))
        throw std::runtime_error("Failed to mount SD card");
#endif

    cardType = SDLIB.cardType();

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

void SDCard::listFiles(std::function<void(const std::string&)> fileCallback)
{
    this->listFiles("/", fileCallback);
}

void SDCard::listFiles(const std::string& path, std::function<void(const std::string&)> fileCallback)
{
    this->mountOrThrow();

    File root = SDLIB.open(path.c_str());

    if (!root)
    {
        Log::println("SDCARD", "Failed to open directory: %s", path.c_str());
        return;
    }

    if (!root.isDirectory())
    {
        Log::println("SDCARD", "Failed to list: %s is not a directory!", path.c_str());
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.name()[0] == '.')
        {
            file = root.openNextFile();
            continue;
        }

        std::string fullPath = path;

        if (fullPath.back() != '/')
            fullPath += "/";
        
        fullPath += file.name();
        
        if (!file.isDirectory())
            fileCallback(fullPath);
        else
            listFiles(fullPath, fileCallback); // Recursive call for subdirectories

        file = root.openNextFile();
    }

    root.close();
}

std::string SDCard::nextFile(std::string dir, int skip)
{
    this->mountOrThrow();

    File root = SDLIB.open(dir.c_str());

    if (!root)
    {
        Log::println("SDCARD", "Failed to open directory %s!", dir.c_str());
        return "";
    }

    if (!root.isDirectory())
    {
        Log::println("SDCARD", "Failed to list %s: not a directory!", dir.c_str());
        return "";
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory() || file.name()[0] == '.')
        {
            file = root.openNextFile();
            continue;
        }

        if(skip > 0)
        {
            skip--;
            file = root.openNextFile();
            continue;
        }

        return file.path();
    }

    root.close();

    return "";
}

int SDCard::countFiles(std::string dir)
{
    this->mountOrThrow();

    File root = SDLIB.open(dir.c_str());

    if (!root)
    {
        Log::println("SDCARD", "Failed to open directory %s!", dir.c_str());
        return 0;
    }

    if (!root.isDirectory())
    {
        Log::println("SDCARD", "Failed to list %s: not a directory!", dir.c_str());
        root.close();
        return 0;
    }

    int count = 0;
    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory() || file.name()[0] == '.')
        {
            file = root.openNextFile();
            continue;
        }

        count++;
        file = root.openNextFile();
    }

    root.close();
    return count;
}

bool SDCard::fileExists(const std::string filename) 
{
    this->mountOrThrow();
    return SDLIB.exists(filename.c_str());
}

void SDCard::writeJsonFile(const std::string filename, JsonDocument& jsonDocument)
{
    this->mountOrThrow();

    File file = SDLIB.open(filename.c_str(), FILE_WRITE);
    if (!file)
        throw std::runtime_error("Failed to create file");

    serializeJsonPretty(jsonDocument, file);

    Log::println("SDCARD", "JSON file created: %s", filename.c_str());
    file.close();
}

void SDCard::writeTextFile(const std::string filename, const char* text)
{
    this->mountOrThrow();

    File file = SDLIB.open(filename.c_str(), FILE_WRITE);
    if (!file)
        throw std::runtime_error("Failed to create file");

    file.print(text);

    Log::println("SDCARD", "Text file created: %s", filename.c_str());
    file.close();
}

void SDCard::readParseJsonFile(const std::string filename, JsonDocument& targetJsonDocument)
{
    this->mountOrThrow();

    File file = SDLIB.open(filename.c_str());
    if (!file)
        throw std::runtime_error("Failed to open file");

    DeserializationError error = deserializeJson(targetJsonDocument, file);

    file.close();

    if (error != DeserializationError::Ok)
        throw std::runtime_error(error.c_str());
}

size_t SDCard::getSectorCount()
{
    this->mountOrThrow();
    return SDLIB.numSectors();
}

size_t SDCard::getSectorSize()
{
    this->mountOrThrow();
    return SDLIB.sectorSize();
}
