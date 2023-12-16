#include "log.h"
#include "SD.h"
#include "config.h"

#include "sdcard.h"

SPIClass sdSpi;

SDCard::SDCard()
{
    sdSpi.begin(GPIO_SD_SCK, GPIO_SD_DO, GPIO_SD_DI, -1);
}

void SDCard::listFiles()
{
    if (!SD.begin(GPIO_SD_CS, sdSpi))
    {
        Log::println("SDCARD", "Card mount failed");
        return;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Log::println("SDCARD", "No SD card attached");
        return;
    }

    if (cardType == CARD_MMC)
        Log::println("SDCARD", "Card type MMC");
    else if (cardType == CARD_SD)
        Log::println("SDCARD", "Card type SD");
    else if (cardType == CARD_SDHC)
        Log::println("SDCARD", "Card type SDHC");
    else
        Log::println("SDCARD", "Card type UNKNOWN");

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
            Log::println("SDCARD", "  DIR: %s", file.name());
        else
            Log::println("SDCARD", "  FILE: %s - SIZE: %d", file.name(), file.size());

        file = root.openNextFile();
    }
}
