#include "log.h"
#include "utils.h"
#include "TAS5806.h"

using namespace std;

TAS5806::TAS5806(shared_ptr<TwoWire> wire, uint8_t deviceAddress)
{
    this->wire = wire;
    this->deviceAddress = deviceAddress;
}

void TAS5806::resetChip()
{
    // 7.6.1.1 RESET_CTRL Register (Offset = 1h) [reset = 0x00]
    uint8_t registerAddress = 0x01;
    uint8_t registerValue = 0b00010001;
    //                           |   |
    //                           |   reset all registers
    //                           reset module
    auto err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    if (err)
        Log::println("TAS5806", "ERROR! Reset chip failed: %d", err);

    // // 7.6.1.2 DEVICE_CTRL_1 Register (Offset = 2h) [reset = 0x00]
    // registerAddress = 0x02;
    // registerValue = 0b0000000;
    // //                 |˩˩||˩
    // //                 |  |00 => BD Mode
    // //                 |  0 => BTL Mode
    // //                 000 => 768K

    // err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    // if(err)
    //     Log::println("TAS5806", "ERROR! Set DEVICE_CTRL_1 register failed: %d", err);
}

void TAS5806::setParamsAndHighZ()
{
    // Params hard-coded for now... not a very universal library at the moment ;)

    // 7.6.1.3 DEVICE_CTRL_2 Register (Offset = 3h) [reset = 0x10]
    uint8_t registerAddress = 0x03;
    uint8_t registerValue = 0b00000010;
    //                           || |˩
    //                           || 10 => CTRL State HIGH-Z
    //                           |0 => unmute
    //                           0 => DSP to "normal operation"

    auto err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    if (err)
        Log::println("TAS5806", "ERROR! Set DEVICE_CTRL_2 register failed: %d", err);

    // 7.6.1.5 SIG_CH_CTRL Register (Offset = 28h) [reset = 0x00]
    registerAddress = 0x28;
    registerValue = 0b00110000;
    //                |˩˩˩|˩˩˩
    //                |   0000 => Auto sample rate detection
    //                0011 => 32FS

    err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    if (err)
        Log::println("TAS5806", "ERROR! Set SIG_CH_CTRL register failed: %d", err);

    // 7.6.1.9 SAP_CTRL1 Register (Offset = 33h) [reset = 0x02]
    registerAddress = 0x33;
    registerValue = 0b00000000;
    //                | |˩|˩|˩
    //                | | | 00 => Word length 16bits
    //                | | 00 => LRCLK pulse normal
    //                | 00 => Data format I2S
    //                0 => I2S shift MSB

    // err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    // if(err)
    //     Log::println("TAS5806", "ERROR! Set SAP_CTRL1 register failed: %d", err);

    // 7.6.1.21 AGAIN Register (Offset = 54h) [reset = 0x00]
    registerAddress = 0x54;
    registerValue = 0b00001100;
    //                   |˩˩˩˩ Analog Gain Control (0.5dB step) 00000: 0 dB, 00001: -0.5db, 11111: -15.5 dB
    //                         1100 => -6dB

    err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    if (err)
        Log::println("TAS5806", "ERROR! Set AGAIN register failed: %d", err);

    // // 7.6.1.18 AUTO_MUTE_CTRL Register (Offset = 50h) [reset = 0x07]
    // registerAddress = 0x50;
    // registerValue = 0b00000000;
    // //                     |||
    // //                     ||0: Disable left channel auto mute
    // //                     |0: Disable right channel auto mute
    // //                     0: Auto mute left channel and right channel independently

    // err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    // if (err)
    //     Log::println("TAS5806", "ERROR! Set AUTO_MUTE_CTRL register failed: %d", err);
}

void TAS5806::setModePlay()
{
    // 7.6.1.3 DEVICE_CTRL_2 Register (Offset = 3h) [reset = 0x10]
    uint8_t registerAddress = 0x03;
    uint8_t registerValue = 0b00000011;
    //                           || |˩
    //                           || 10 => CTRL State PLAY
    //                           |0 => unmute
    //                           0 => Dont Reset DSP

    auto err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    if (err)
        Log::println("TAS5806", "ERROR! Set DEVICE_CTRL_2 register failed: %d", err);
}

void TAS5806::setVolume(uint8_t volume)
{
    // 7.6.1.15 DIG_VOL_CTL Register (Offset = 4Ch) [reset = 30h]
    uint8_t registerAddress = 0x4C;
    uint8_t registerValue = 254 - volume; // 254 - volume => 0 = mute, 254 = max volume
    // These bits control both left and right channel digital volume. The
    // digital volume is 24 dB to -103 dB in -0.5 dB step.
    // 00000000: +24.0 dB
    // 00000001: +23.5 dB
    // ........
    // and 00101111: +0.5 dB
    // 00110000: 0.0 dB
    // 00110001: -0.5 dB
    // .......
    // 11111110: -103 dB
    // 11111111: Mute

    auto err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    if (err)
        Log::println("TAS5806", "ERROR! Set DIG_VOL_CTL register failed: %d", err);
    else
        Log::println("TAS5806", "DIG_VOL_CTL set to: %X02", registerValue);

}

void TAS5806::readPrintBinaryRegister(uint8_t addr, const char *name, uint8_t expected)
{
    uint8_t buffer[1];
    char bufferBin[9];
    auto err = Utils::readI2CRegister(this->wire, this->deviceAddress, addr, buffer, 1);

    if (err)
        Log::println("TAS5806", "ERROR READING %s [0x%02X], err: %d", name, addr, err);
    else if (buffer[0] != expected)
    {
        itoa(buffer[0], bufferBin, 2);
        Log::println("TAS5806", "%s [0x%02X] NOT EXPECTED: %s", name, addr, bufferBin);
    }
    else
        Log::println("TAS5806", "%s [0x%02X] OK", name, addr);
}

void TAS5806::readPrintValueRegister(uint8_t addr, const char *name, uint8_t expected)
{
    uint8_t buffer[1];
    auto err = Utils::readI2CRegister(this->wire, this->deviceAddress, addr, buffer, 1);
    if (err)
        Log::println("TAS5806", "ERROR READING %s [0x%02X], err: %d", name, addr, err);
    else if (buffer[0] != expected)
        Log::println("TAS5806", "%s [0x%02X] NOT EXPECTED: %d", name, addr, buffer[0]);
    // else
    //     Log::println("TAS5806", "%s [0x%02X] OK", name, addr);
}

void TAS5806::setMute(bool mute)
{
    // 7.6.1.3 DEVICE_CTRL_2 Register (Offset = 3h) [reset = 0x10]
    uint8_t registerAddress = 0x03;

    uint8_t buffer[1];
    auto err = Utils::readI2CRegister(this->wire, this->deviceAddress, registerAddress, buffer, 1);
    if (err) {
        Log::println("TAS5806", "ERROR! Read DEVICE_CTRL_2 register failed: %d", err);
        return;
    }

    uint8_t registerValue = buffer[0];
    if(mute)
        registerValue |= 0b00001000;
    else
        registerValue &= 0b11110111;
    //                         |
    //                         1 => mute

    err = Utils::writeI2CRegister(this->wire, this->deviceAddress, registerAddress, registerValue);
    if (err)
        Log::println("TAS5806", "ERROR! Set DEVICE_CTRL_2 register failed: %d", err);
    else
        Log::println("TAS5806", "DEVICE_CTRL_2 set to: %X02", registerValue);
}

void TAS5806::printMonRegisters()
{
    // FS Mon depends on sample rate
    // // 7.6.1.12 FS_MON Register (Offset = 37h) [reset = 0x00]
    // this->readPrintBinaryRegister(0x37, "FS_MON", 0b00000110); // expected 32KHz

    // 7.6.1.13 BCK_MON Register (Offset = 38h) [reset = 0x00]
    this->readPrintValueRegister(0x38, "BCK_MON", 32); // expected 32FS

    // 7.6.1.14 CLKDET_STATUS Register (Offset = 39h) [reset = 0x00]
    this->readPrintBinaryRegister(0x39, "CLKDET_STATUS", 0b00001000); // PLL locked, others 0

    // 7.6.1.15 DIG_VOL_CTL Register (Offset = 4Ch) [reset = 30h]
    this->readPrintBinaryRegister(0x4C, "DIG_VOL_CTL", 0b00001000);

    // 7.6.1.36 CHAN_FAULT Register (Offset = 70h) [reset = 0x00]
    this->readPrintBinaryRegister(0x70, "CHAN_FAULT", 0x00);

    // 7.6.1.37 GLOBAL_FAULT1 Register (Offset = 71h) [reset = 0h]
    this->readPrintBinaryRegister(0x71, "GLOBAL_FAULT1", 0x00);

    // 7.6.1.38 GLOBAL_FAULT2 Register (Offset = 72h) [reset = 0h]
    this->readPrintBinaryRegister(0x72, "GLOBAL_FAULT2", 0x00);

    // 7.6.1.39 OT WARNING Register (Offset = 73h) [reset = 0x00]
    this->readPrintBinaryRegister(0x73, "OT_WARNING", 0x00);

    // 7.6.1.40 PIN_CONTROL1 Register (Offset = 74h) [reset = 0x00]
    this->readPrintBinaryRegister(0x74, "PIN_CONTROL1", 0x00);

    // 7.6.1.41 PIN_CONTROL2 Register (Offset = 75h) [reset = 0xF8]
    this->readPrintBinaryRegister(0x75, "PIN_CONTROL2", 0b00111000);

    // 7.6.1.42 MISC_CONTROL Register (Offset = 76h) [reset = 0x00]
    this->readPrintBinaryRegister(0x76, "MISC_CONTROL", 0x00);

    // 7.6.1.28 POWER_STATE Register (Offset = 68h) [reset = 0x00]
    this->readPrintValueRegister(0x68, "POWER_STATE", 0x03); // Play

    // 7.6.1.29 AUTOMUTE_STATE Register (Offset = 69h) [reset = 0x00]
    this->readPrintBinaryRegister(0x69, "AUTOMUTE_STATE", 0x00);
}