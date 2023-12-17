#pragma once

// Task priorities
#define TASK_PRIO_HBI_WORKER 1
#define TASK_STACK_SIZE_HBI_WORKER 10000

// Well known SDCARD files
#define SDCARD_FILE_WIFI_CONFIG "/wifi.json"
#define SDCARD_FILE_HBI_CONFIG "/hbi.json"
#define SDCARD_FILE_AUDIO_CONFIG "/audio.json"

// I2C pins
#define GPIO_I2C_SDA 10
#define GPIO_I2C_SCL 11

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

// Power pins
#define GPIO_POWER_3V3_NPSAVE 46
#define GPIO_POWER_12V_ENABLE 45

// HBI pins
#define GPIO_HBI_INPUT_INT 15
#define GPIO_HBI_LEDDRIVER_RST 14
#define GPIO_HBI_ENCODER_A 16
#define GPIO_HBI_ENCODER_B 17
#define GPIO_HBI_ENCODER_BTN 18

// Audio pins
#define GPIO_AUDIO_BCLK 2
#define GPIO_AUDIO_LRCLK 3
#define GPIO_AUDIO_DOUT 4
#define GPIO_AUDIO_CODEC_NPDN 1

// SD card pins
#define GPIO_SD_SCK 36
#define GPIO_SD_DO 37
#define GPIO_SD_DI 35
#define GPIO_SD_CS 34
#define GPIO_SD_DETECT 26