
#pragma once

// ST7735 driver for Retro-Go
// Reuses the existing ILI9341 SPI/DMA implementation.

#define lcd_set_window st7735_original_window
#define lcd_init st7735_original_init

#include "ili9341.h"

#undef lcd_set_window
#undef lcd_init

// Default offsets for a 128x160 ST7735 module.
// Adjust after testing if the image is shifted.
#ifndef RG_ST7735_X_OFFSET
#define RG_ST7735_X_OFFSET 0
#endif

#ifndef RG_ST7735_Y_OFFSET
#define RG_ST7735_Y_OFFSET 0
#endif

static void lcd_set_window(
    int left, int top, int width, int height)
{
    int x0 = left + RG_ST7735_X_OFFSET;
    int y0 = top + RG_ST7735_Y_OFFSET;
    int x1 = x0 + width - 1;
    int y1 = y0 + height - 1;

    ILI9341_CMD(0x2A,
        x0 >> 8, x0 & 0xFF,
        x1 >> 8, x1 & 0xFF);

    ILI9341_CMD(0x2B,
        y0 >> 8, y0 & 0xFF,
        y1 >> 8, y1 & 0xFF);

    ILI9341_CMD(0x2C);
}

static void lcd_init(void)
{
    spi_init();

    gpio_set_direction(
        RG_GPIO_LCD_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(RG_GPIO_LCD_DC, 1);

    gpio_set_direction(
        RG_GPIO_LCD_RST, GPIO_MODE_OUTPUT);

    gpio_set_level(RG_GPIO_LCD_RST, 0);
    rg_usleep(100000);

    gpio_set_level(RG_GPIO_LCD_RST, 1);
    rg_usleep(120000);

    // Software reset
    ILI9341_CMD(0x01);
    rg_usleep(150000);

    // Sleep out
    ILI9341_CMD(0x11);
    rg_usleep(150000);

    // Frame rate control
    ILI9341_CMD(0xB1, 0x01, 0x2C, 0x2D);
    ILI9341_CMD(0xB2, 0x01, 0x2C, 0x2D);
    ILI9341_CMD(0xB3,
        0x01, 0x2C, 0x2D,
        0x01, 0x2C, 0x2D);

    ILI9341_CMD(0xB4, 0x07);

    // Power control
    ILI9341_CMD(0xC0, 0xA2, 0x02, 0x84);
    ILI9341_CMD(0xC1, 0xC5);
    ILI9341_CMD(0xC2, 0x0A, 0x00);
    ILI9341_CMD(0xC3, 0x8A, 0x2A);
    ILI9341_CMD(0xC4, 0x8A, 0xEE);
    ILI9341_CMD(0xC5, 0x0E);

    // Inversion off
    ILI9341_CMD(0x20);

    // RGB565
    ILI9341_CMD(0x3A, 0x05);

    // Landscape orientation
    // MV + MX + BGR
    ILI9341_CMD(0x36, 0x68);

    // Normal display mode
    ILI9341_CMD(0x13);
    rg_usleep(10000);

    // Display ON
    ILI9341_CMD(0x29);
    rg_usleep(100000);
}
