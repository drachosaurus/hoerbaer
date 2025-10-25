#pragma once

#ifndef PINOUT_PCB_REV
// PCB Rev 2 (latest) is the default
#define PINOUT_PCB_REV 2
#endif

// Task priorities
#define TASK_PRIO_HBI_WORKER 2
#define TASK_STACK_SIZE_HBI_WORKER_WORDS (30 * 1024 / 4) // 30 kbytes
#define TASK_PRIO_BLE_WORKER 1
#define TASK_STACK_SIZE_BLE_WORKER_WORDS (10 * 1024 / 4) // 10 kbytes
#define TASK_PRIO_RFID_WORKER 2
#define TASK_STACK_SIZE_RFID_WORKER_WORDS (10 * 1024 / 4) // 10 kbytes

// Well known SDCARD files
#define SDCARD_FILE_CONFIG "/config.json"
#define SDCARD_FILE_META_CACHE "/_metaCache.json"

// BLE IDs
#define BLE_SERVICE_UUID "4ed1ce10-a038-404e-9e93-64bc8d8a4753"
#define BLE_CHARACTERISTIC_POWER_UUID "bdb1d967-8a30-42fd-b035-0b65e15074ca"
#define BLE_CHARACTERISTIC_PLAYER_UUID "936bc2e0-2ba8-4a15-9c98-b82fcc308d22"
#define BLE_CHARACTERISTIC_NETWORK_UUID "14fbff44-b62b-4f75-91aa-aac6df208754"
#define BLE_CHARACTERISTIC_CONTROL_UUID "e3a1c5f0-7b2d-4c8a-9f3e-2d6b8a9e5c4f"
#define BLE_CHARACTERISTIC_PLAYER_CMD_UUID "f7a12580-4bc8-46c5-9f69-d7935c3a2b01"

// BLE characteristics update interval
#define BLE_CHARACTERISTICS_UPDATE_INTERVAL_MILLIS 1000

// Audio playing info update interval
#define AUDIO_PLAYING_INGO_UPDATE_INTERVAL_MILLIS 500

// I2C addresses
#define I2C_ADDR_LED_DRIVER1 0x40   // (R: 0x81, W: 0x80)
#define I2C_ADDR_LED_DRIVER2 0x41   // (R: 0x83, W: 0x82)
#define I2C_ADDR_LED_DRIVER3 0x42   // (R: 0x85, W: 0x84)
#define I2C_ADDR_IO_EXPANDER1 0x38  // (R: 0x71, W: 0x70)
#define I2C_ADDR_IO_EXPANDER2 0x39  // (R: 0x73, W: 0x72)
#define I2C_ADDR_IO_EXPANDER3 0x3A  // (R: 0x75, W: 0x74)
#define I2C_ADDR_AUDIO_CODEC 0x2C   // (R: 0x59, W: 0x58)
// 0x36 (R: 0x6d, W: 0x6c) -> Fuel gauge, set in the  MAX17048 library


// TODO: who is this?
// I2C device found at address 0x48 (R: 0x91, W: 0x90)
// I2C device found at address 0x4b (R: 0x97, W: 0x96)

// Power management
#define POWER_BATTERY_CHECK_INTERVAL_MILLIS 5000
#define POWER_SHUTDOWN_VOLTAGE 3.0f
#define POWER_PERIPHERIAL_STARTUP_DELAY 300

// PINOUT -----------------------------

#if PINOUT_PCB_REV == 2

    // I2C pins
    #define GPIO_I2C_SDA 10
    #define GPIO_I2C_SCL 11

    // Power pins
    #define GPIO_POWER_3V3_NPSAVE 45
    #define GPIO_POWER_HV_ENABLE 42
    #define GPIO_POWER_VCC_P_ENABLE 48
    #define GPIO_POWER_CHG_STAT 47

    // HBI pins
    #define GPIO_HBI_INPUT_INT 7
    #define GPIO_HBI_LEDDRIVER_RST 15 
    #define GPIO_HBI_ENCODER_A 4
    #define GPIO_HBI_ENCODER_B 5
    #define GPIO_HBI_ENCODER_BTN 6

    // RFID pins (SPI)
    #define GPIO_RFID_SS 8
    #define GPIO_RFID_MOSI 16
    #define GPIO_RFID_CLK 17
    #define GPIO_RFID_MISO 18
    #define GPIO_RFID_RST 2

    // Audio pins
    #define GPIO_AUDIO_BCLK 46
    #define GPIO_AUDIO_LRCLK 3
    #define GPIO_AUDIO_DOUT 9
    #define GPIO_AUDIO_CODEC_NPDN 14

    // SD card pins
    #ifndef SD_MODE_SDMMC
        #define SD_MODE_SDMMC
    #endif
    #define SDMMC_FREQ 40000
    #define GPIO_SD_CLK 13
    #define GPIO_SD_CMD 12
    #define GPIO_SD_D0 38
    #define GPIO_SD_D1 39
    #define GPIO_SD_D2 40
    #define GPIO_SD_D3 41
    #define GPIO_SD_DETECT 21

#endif
#if PINOUT_PCB_REV == 1

    // I2C pins
    #define GPIO_I2C_SDA 10
    #define GPIO_I2C_SCL 11

    // Power pins
    #define GPIO_POWER_3V3_NPSAVE 46
    #define GPIO_POWER_HV_ENABLE 45
    #define GPIO_POWER_CHG_STAT 48

    // HBI pins
    #define GPIO_HBI_INPUT_INT 15
    #define GPIO_HBI_LEDDRIVER_RST 14
    #define GPIO_HBI_ENCODER_A 16
    #define GPIO_HBI_ENCODER_B 17
    #define GPIO_HBI_ENCODER_BTN 18

    // RFID pins (SPI)
    #define GPIO_RFID_SS 8
    #define GPIO_RFID_MOSI 16
    #define GPIO_RFID_CLK 17
    #define GPIO_RFID_MISO 18
    #define GPIO_RFID_RST 2

    // Audio pins
    #define GPIO_AUDIO_BCLK 2
    #define GPIO_AUDIO_LRCLK 3
    #define GPIO_AUDIO_DOUT 4
    #define GPIO_AUDIO_CODEC_NPDN 1

    // SD card pins
    #ifndef SD_MODE_SPI
        #define SD_MODE_SPI
    #endif
    #define GPIO_SD_SCK 36
    #define GPIO_SD_DO 37
    #define GPIO_SD_DI 35
    #define GPIO_SD_CS 34
    #define GPIO_SD_DETECT 26

#endif

