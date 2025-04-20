#pragma once

#include "sdcard.h"

class ID3Parser {
    private:
        static std::string convertUTF16ToUTF8(const char* utf16Buffer, size_t bufferLength, bool hasBOM);
        static std::string readID3v2FrameContent(File &file, uint32_t frameSize, uint8_t encoding);
        static uint32_t readSynchsafeInteger(File &file);

    public:
        static std::tuple<std::string, std::string> readId3Tags(FSTYPE& fs, const std::string& filePath);
};