#include "log.h"
#include "id3parser.h"

// Function to convert ISO-8859-1 (Latin-1) to UTF-8
std::string ID3Parser::convertISO8859_1ToUTF8(const char* iso8859_1Buffer, size_t bufferLength) {
    std::string result;
    result.reserve(bufferLength * 2); // Reserve space (worst case: each char becomes 2 bytes)

    for (size_t i = 0; i < bufferLength && iso8859_1Buffer[i] != '\0'; i++) {
        unsigned char ch = (unsigned char)iso8859_1Buffer[i];
        
        if (ch < 0x80) {
            // ASCII character (0x00-0x7F) - single byte in UTF-8
            result.push_back((char)ch);
        } else {
            // Extended ASCII (0x80-0xFF) - two bytes in UTF-8
            result.push_back((char)(0xC0 | (ch >> 6)));
            result.push_back((char)(0x80 | (ch & 0x3F)));
        }
    }

    return result;
}

// Function to convert UTF-16 to UTF-8
std::string ID3Parser::convertUTF16ToUTF8(const char* utf16Buffer, size_t bufferLength, bool hasBOM) {
    std::string result;
    size_t i = 0;

    // Skip BOM if present
    if (hasBOM && bufferLength >= 2) {
        i = 2; // Skip the first 2 bytes (BOM)
    }

    bool bigEndian = false;
    // Check BOM to determine endianness
    if (hasBOM && bufferLength >= 2) {
        if ((unsigned char)utf16Buffer[0] == 0xFE && (unsigned char)utf16Buffer[1] == 0xFF) {
            bigEndian = true;
        }
        // If BOM is 0xFF 0xFE, it's little endian (default)
    }

    // Process the UTF-16 characters
    while (i + 1 < bufferLength) {
        uint16_t utf16Char;

        // Get the 16-bit character based on endianness
        if (bigEndian) {
            utf16Char = ((unsigned char)utf16Buffer[i] << 8) | (unsigned char)utf16Buffer[i + 1];
        }
        else {
            utf16Char = ((unsigned char)utf16Buffer[i + 1] << 8) | (unsigned char)utf16Buffer[i];
        }

        i += 2; // Move to the next UTF-16 character

        // Convert UTF-16 to UTF-8
        if (utf16Char < 0x80) {
            // 1-byte UTF-8 character
            result.push_back((char)utf16Char);
        }
        else if (utf16Char < 0x800) {
            // 2-byte UTF-8 character
            result.push_back((char)(0xC0 | (utf16Char >> 6)));
            result.push_back((char)(0x80 | (utf16Char & 0x3F)));
        }
        else if (utf16Char >= 0xD800 && utf16Char <= 0xDBFF && i + 1 < bufferLength) {
            // This is a surrogate pair (for characters outside the BMP)
            uint16_t lowSurrogate;
            if (bigEndian) {
                lowSurrogate = ((unsigned char)utf16Buffer[i] << 8) | (unsigned char)utf16Buffer[i + 1];
            }
            else {
                lowSurrogate = ((unsigned char)utf16Buffer[i + 1] << 8) | (unsigned char)utf16Buffer[i];
            }

            i += 2; // Skip the low surrogate

            if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
                // Valid surrogate pair
                uint32_t codePoint = 0x10000 + (((utf16Char - 0xD800) << 10) | (lowSurrogate - 0xDC00));

                // 4-byte UTF-8 character
                result.push_back((char)(0xF0 | (codePoint >> 18)));
                result.push_back((char)(0x80 | ((codePoint >> 12) & 0x3F)));
                result.push_back((char)(0x80 | ((codePoint >> 6) & 0x3F)));
                result.push_back((char)(0x80 | (codePoint & 0x3F)));
            }
        }
        else if (!(utf16Char >= 0xD800 && utf16Char <= 0xDFFF)) {
            // Regular 3-byte UTF-8 character (not a surrogate)
            result.push_back((char)(0xE0 | (utf16Char >> 12)));
            result.push_back((char)(0x80 | ((utf16Char >> 6) & 0x3F)));
            result.push_back((char)(0x80 | (utf16Char & 0x3F)));
        }
        // Skip invalid surrogates
    }

    return result;
}

// Helper function to read synchsafe integers (used in ID3v2)
uint32_t ID3Parser::readSynchsafeInteger(File& file) {
    uint8_t buffer[4];
    file.readBytes((char*)buffer, 4);

    // Convert from synchsafe integer (7 bits per byte)
    uint32_t result = 0;
    result = buffer[0] << 21 |
        buffer[1] << 14 |
        buffer[2] << 7 |
        buffer[3];
    return result;
}

// Helper function to read ID3v2 frame content with encoding
std::string ID3Parser::readID3v2FrameContent(File& file, uint32_t frameSize, uint8_t encoding) {
    if (frameSize == 0) return "";

    // First byte is text encoding
    char* buffer = new char[frameSize + 1];
    if (!buffer) return ""; // Memory allocation failed

    // Read the frame content
    file.readBytes(buffer, frameSize);
    buffer[frameSize] = '\0';

    std::string result;

    // Handle different encodings
    switch (encoding) {
        case 0: // ISO-8859-1 (Latin-1)
            result = convertISO8859_1ToUTF8(buffer, frameSize);
            break;
        case 1: // UTF-16 with BOM
            // Convert UTF-16 with BOM to UTF-8
            result = convertUTF16ToUTF8(buffer, frameSize, true);
            break;

        case 2: // UTF-16BE without BOM
            // Convert UTF-16BE without BOM to UTF-8
            // We pretend there's a BE BOM by passing appropriate params
            result = convertUTF16ToUTF8(buffer, frameSize, false);
            break;

        case 3: // UTF-8
            result = std::string(buffer);
            break;

        default:
            result = std::string(buffer);
    }

    delete[] buffer;
    return result;
}


std::tuple<std::string, std::string> ID3Parser::readId3Tags(FSTYPE& fs, const std::string& filePath) {

    std::string title = "";
    std::string artist = "";
    // String album = "";

    File mp3File = fs.open(filePath.c_str());
    if (!mp3File) {
        Log::println("ID3", "Metadata: failed to open file: %s", filePath.c_str());
        return std::make_tuple(title, artist);
    }

    bool metadataFound = false;

    // Check for ID3v2 tag first (at beginning of file)
    mp3File.seek(0);
    char id3Header[4];
    mp3File.readBytes(id3Header, 3);
    id3Header[3] = '\0';

    if (strcmp(id3Header, "ID3") == 0) {
        metadataFound = true;

        // Read version and flags
        uint8_t majorVersion = mp3File.read();
        uint8_t minorVersion = mp3File.read();
        uint8_t flags = mp3File.read();

        // Read tag size (synchsafe integer)
        uint32_t tagSize = readSynchsafeInteger(mp3File);

        // Skip extended header if present
        if (flags & 0x40) {
            uint32_t extHeaderSize = readSynchsafeInteger(mp3File);
            mp3File.seek(mp3File.position() + extHeaderSize - 4);
        }

        // Read frames until we reach the end of the tag
        uint32_t bytesRead = 10; // Header is 10 bytes

        while (bytesRead < tagSize) {
            // Read frame ID (4 bytes)
            char frameID[5];
            mp3File.readBytes(frameID, 4);
            frameID[4] = '\0';
            bytesRead += 4;

            // Break if we've reached padding (indicated by a null byte)
            if (frameID[0] == 0) break;

            // Read frame size
            uint32_t frameSize;
            if (majorVersion >= 4) {
                frameSize = readSynchsafeInteger(mp3File);
            }
            else {
                uint8_t sizeBuf[4];
                mp3File.readBytes((char*)sizeBuf, 4);
                frameSize = (sizeBuf[0] << 24) | (sizeBuf[1] << 16) | (sizeBuf[2] << 8) | sizeBuf[3];
            }
            bytesRead += 4;

            // Read frame flags (2 bytes)
            uint8_t frameFlags[2];
            mp3File.readBytes((char*)frameFlags, 2);
            bytesRead += 2;

            // Process known frames
            if (strcmp(frameID, "TIT2") == 0) {
                // Title frame
                uint8_t encoding = mp3File.read();
                bytesRead++;
                title = readID3v2FrameContent(mp3File, frameSize - 1, encoding);
                bytesRead += frameSize - 1;
            }
            else if (strcmp(frameID, "TPE1") == 0) {
                // Artist frame
                uint8_t encoding = mp3File.read();
                bytesRead++;
                artist = readID3v2FrameContent(mp3File, frameSize - 1, encoding);
                bytesRead += frameSize - 1;
            }
            // else if (strcmp(frameID, "TALB") == 0) {
            //   // Album frame
            //   uint8_t encoding = mp3File.read();
            //   bytesRead++;
            //   album = readID3v2FrameContent(mp3File, frameSize - 1, encoding);
            //   bytesRead += frameSize - 1;
            // } 
            else {
                // Skip unknown frames
                mp3File.seek(mp3File.position() + frameSize);
                bytesRead += frameSize;
            }
        }
    }

    // Check for ID3v1 tag if needed (as fallback or additional info)
    if (!metadataFound || title.empty() || artist.empty()) {
        if (mp3File.size() > 128) {
            mp3File.seek(mp3File.size() - 128);

            char tag[4];
            mp3File.readBytes(tag, 3);
            tag[3] = '\0';

            if (strcmp(tag, "TAG") == 0) {
                metadataFound = true;

                // Read ID3v1 fields
                char titleBuffer[31];
                char artistBuffer[31];
                char albumBuffer[31];

                mp3File.readBytes(titleBuffer, 30);
                titleBuffer[30] = '\0';

                mp3File.readBytes(artistBuffer, 30);
                artistBuffer[30] = '\0';

                mp3File.readBytes(albumBuffer, 30);
                albumBuffer[30] = '\0';

                // Use ID3v1 data only if ID3v2 didn't provide it
                // ID3v1 uses ISO-8859-1 encoding, convert to UTF-8
                if (title.empty()) title = convertISO8859_1ToUTF8(titleBuffer, 30);
                if (artist.empty()) artist = convertISO8859_1ToUTF8(artistBuffer, 30);
                //   if (album.empty()) album = std::string(albumBuffer);
            }
        }
    }

    // Close the file
    mp3File.close();

    return std::make_tuple(title, artist);
}