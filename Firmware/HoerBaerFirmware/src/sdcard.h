#pragma once

#include <ArduinoJson.h>

#ifdef SD_MODE_SDMMC
#include <SD_MMC.h>
#define FSTYPE fs::SDMMCFS
#else 
#include <SD.h>
#define FSTYPE fs::SDFS
#endif

class SDCard {
    private:
        bool cardMounted = false;
        void mountOrThrow();
    public:
        SDCard();
        FSTYPE& getFs();
        bool cardPresent();
        bool fileExists(const std::string filename);
        void writeJsonFile(const std::string filename, JsonDocument& jsonDocument);
        void writeTextFile(const std::string filename, const char* text);
        void listFiles(std::function<void(const std::string&)> fileCallback);
        void listFiles(const std::string& path, std::function<void(const std::string&)> fileCallback);
        std::string nextFile(std::string dir, int skip);
        int countFiles(std::string dir);
        void readParseJsonFile(const std::string filename, JsonDocument& targetJsonDocument);
        size_t getSectorCount();
        size_t getSectorSize();
};
