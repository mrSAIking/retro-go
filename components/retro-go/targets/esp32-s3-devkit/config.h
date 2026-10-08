
#pragma once

// ESP32-S3 N16R8 + ST7735 1.8-inch TFT
#define RG_TARGET_NAME "ESP32-S3-N16R8-ST7735"

// Storage - separate microSD SPI module
#define RG_STORAGE_ROOT "/sd"
#define RG_STORAGE_SDSPI_HOST SPI3_HOST
#define RG_STORAGE_SDSPI_SPEED SDMMC_FREQ_DEFAULT

// Audio - no speaker or DAC connected
#define RG_AUDIO_USE_INT_DAC 0
#define RG_AUDIO_USE_EXT_DAC 0

// Video - ST7735 landscape 160x128
#define RG_SCREEN_DRIVER 2
#define RG_SCREEN_HOST SPI2_HOST
#define RG_SCREEN_SPEED 20000000
#define RG_SCREEN_BACKLIGHT 0
#define RG_SCREEN_WIDTH 160
#define RG_SCREEN_HEIGHT 128
#define RG_SCREEN_ROTATE 0
#define RG_SCREEN_VISIBLE_AREA {0, 0, 0, 0}
#define RG_SCREEN_SAFE_AREA {0, 0, 0, 0}

// ST7735 offsets - adjust if necessary after testing
#define RG_ST7735_X_OFFSET 0
#define RG_ST7735_Y_OFFSET 0

// Buttons - active LOW, internal pull-ups
// No ADC buttons are connected
#define RG_GAMEPAD_ADC_MAP {}

#define RG_GAMEPAD_GPIO_MAP { \
    {RG_KEY_UP,     .num = GPIO_NUM_6,  .pullup = 1, .level = 0}, \
    {RG_KEY_DOWN,   .num = GPIO_NUM_7,  .pullup = 1, .level = 0}, \
    {RG_KEY_LEFT,   .num = GPIO_NUM_8,  .pullup = 1, .level = 0}, \
    {RG_KEY_RIGHT,  .num = GPIO_NUM_9,  .pullup = 1, .level = 0}, \
    {RG_KEY_A,      .num = GPIO_NUM_15, .pullup = 1, .level = 0}, \
    {RG_KEY_B,      .num = GPIO_NUM_16, .pullup = 1, .level = 0}, \
    {RG_KEY_START,  .num = GPIO_NUM_17, .pullup = 1, .level = 0}, \
    {RG_KEY_SELECT, .num = GPIO_NUM_14, .pullup = 1, .level = 0}, \
}

// Battery - no battery measurement connected
#define RG_BATTERY_DRIVER 0

// TFT ST7735 SPI
#define RG_GPIO_LCD_MISO GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI GPIO_NUM_21
#define RG_GPIO_LCD_CLK  GPIO_NUM_18
#define RG_GPIO_LCD_CS   GPIO_NUM_5
#define RG_GPIO_LCD_DC   GPIO_NUM_4
#define RG_GPIO_LCD_RST  GPIO_NUM_3

// TFT LED is connected directly to 3.3V.
// No GPIO-controlled backlight.

// Separate microSD SPI module
#define RG_GPIO_SDSPI_MISO GPIO_NUM_10
#define RG_GPIO_SDSPI_MOSI GPIO_NUM_11
#define RG_GPIO_SDSPI_CLK  GPIO_NUM_12
#define RG_GPIO_SDSPI_CS   GPIO_NUM_13

// No external I2S DAC or status LED configured.
