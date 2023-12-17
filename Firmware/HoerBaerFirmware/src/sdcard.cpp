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

void SDCard::checkCreateFile(const std::string filename, const std::string_view& content)
{
    this->mountOrThrow();

    if (SD.exists(filename.c_str()))
    {
        Log::println("SDCARD", "File already exists: %s", filename.c_str());
        return;
    }

    File file = SD.open(filename.c_str(), FILE_WRITE);
    if (!file)
        throw std::runtime_error("Failed to create file");

    if (!file.print(content.data()))
        throw std::runtime_error("Failed to write content to file");

    Log::println("SDCARD", "File created: %s", filename.c_str());
    file.close();
}
