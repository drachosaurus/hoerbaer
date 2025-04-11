#include <Arduino.h>
#include "log.h"
#include "USB.h"
#include "USBMSC.h"

#include "usb_msc.h"

// TODO: find out from SD card
#define DISK_SECTOR_SIZE 512

// ugly globals... missing context in callbacks
static USBMSC msc;
static FSTYPE* usbmsc_sd;
static size_t usbmc_sdSectorCount;
static size_t usbmc_sdSectorSize;

int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
bool onStartStop(uint8_t power_condition, bool start, bool load_eject);

USBStorage::USBStorage(shared_ptr<SDCard> sdCard) {
    usbmsc_sd = &sdCard->getFs();
    usbmc_sdSectorCount = sdCard->getSectorCount();
    usbmc_sdSectorSize = sdCard->getSectorSize();
}

void USBStorage::initialize() {
    // Initialize USB Mass Storage
    msc.vendorID("ESP32");
    msc.productID("SD Card");
    msc.productRevision("1.0");
    msc.onRead(onRead);
    msc.onWrite(onWrite);
    msc.onStartStop(onStartStop);
    msc.mediaPresent(true);
    USB.begin();

    if(!msc.begin(usbmc_sdSectorCount, usbmc_sdSectorSize))
        throw std::runtime_error("Failed to initialize USB Mass Storage");

    Log::println("USBMSC", "Initialized! Sector count: %i, Sector size: %i", usbmc_sdSectorCount, usbmc_sdSectorSize);
}

// SD card read callback
int32_t onRead(uint32_t startSector, uint32_t offset, void* buffer, uint32_t bufsize) {
    uint8_t *dst = (uint8_t *)buffer;
    size_t sectorsToRead = bufsize / usbmc_sdSectorSize;

    for (int i = 0; i < sectorsToRead; i++)
    {
      if (!usbmsc_sd->readRAW(dst, startSector + i))
      {
        Log::println("USBMSC", "Failed to read sector %d", startSector + i);
        return -1;
      }

      dst += usbmc_sdSectorSize;
    }

    // Log::println("USBMSC", "Read %d sectors from start sector %d", sectorsToRead, startSector);
    return bufsize;
}

  // SD card write callback
int32_t onWrite(uint32_t startSector, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    size_t sectorsToWrite = bufsize / usbmc_sdSectorSize;
    bool res = true;
    for (int i = 0; i < sectorsToWrite; i++)
    {
        res = usbmsc_sd->writeRAW((uint8_t *)buffer, startSector + i);
        if (!res)
        {
            Log::println("USBMSC", "Failed to write sector %d", startSector + i);
            break;
        }

        buffer += usbmc_sdSectorSize;
    }

    // Log::println("USBMSC", "Written %d sectors from start sector %d", sectorsToWrite, startSector);
    return bufsize;
}
  
bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
    Log::println("USBMSC", "StartStop: %d %d %d", power_condition, start, load_eject);
//     if (load_eject)
//     {
//   #ifndef SD_CARD_SPEED_TEST
//       msc.end();
//   #endif
//     }
    return true;
}

