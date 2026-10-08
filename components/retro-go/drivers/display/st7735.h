
#pragma once

// Retro-Go ST7735 SPI display driver
// ESP32-S3 N16R8, 1.8-inch 128x160 TFT
// Landscape resolution: 160x128

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#ifndef RG_ST7735_X_OFFSET
#define RG_ST7735_X_OFFSET 0
#endif

#ifndef RG_ST7735_Y_OFFSET
#define RG_ST7735_Y_OFFSET 0
#endif

static spi_device_handle_t spi_dev;
static QueueHandle_t spi_transactions;
static QueueHandle_t spi_buffers;

#define SPI_TRANSACTION_COUNT 10
#define SPI_BUFFER_COUNT 5
#define SPI_BUFFER_LENGTH (LCD_BUFFER_LENGTH * 2)

// SPI command/data helper
#define ILI9341_CMD(cmd, data...)                    \
    {                                                \
        const uint8_t c = cmd, x[] = {data};         \
        spi_queue_transaction(&c, 1, 0);             \
        if (sizeof(x))                               \
            spi_queue_transaction(x, sizeof(x), 1);  \
    }

static inline uint16_t *spi_take_buffer(void)
{
    uint16_t *buffer;

    if (xQueueReceive(
            spi_buffers,
            &buffer,
            pdMS_TO_TICKS(2500)) != pdTRUE)
    {
        RG_PANIC("display");
    }

    return buffer;
}

static inline void spi_give_buffer(uint16_t *buffer)
{
    xQueueSend(spi_buffers, &buffer, portMAX_DELAY);
}

static inline void spi_queue_transaction(
    const void *data,
    size_t length,
    uint32_t type)
{
    if (!data || !length)
        return;

    spi_transaction_t *t;

    xQueueReceive(
        spi_transactions,
        &t,
        portMAX_DELAY);

    *t = (spi_transaction_t){
        .tx_buffer = NULL,
        .length = length * 8,
        .user = (void *)(uintptr_t)type,
        .flags = 0,
    };

    if (type & 2)
    {
        t->tx_buffer = data;
    }
    else if (length < 5)
    {
        memcpy(t->tx_data, data, length);
        t->flags = SPI_TRANS_USE_TXDATA;
    }
    else
    {
        t->tx_buffer = memcpy(
            spi_take_buffer(),
            data,
            length);

        t->user = (void *)(uintptr_t)(type | 2);
    }

    if (spi_device_queue_trans(
            spi_dev,
            t,
            pdMS_TO_TICKS(2500)) != ESP_OK)
    {
        RG_PANIC("display");
    }
}

IRAM_ATTR
static void spi_pre_transfer_cb(spi_transaction_t *t)
{
    gpio_set_level(
        RG_GPIO_LCD_DC,
        (uintptr_t)t->user & 1);
}

IRAM_ATTR
static void spi_task(void *arg)
{
    spi_transaction_t *t;

    while (spi_device_get_trans_result(
               spi_dev,
               &t,
               portMAX_DELAY) == ESP_OK)
    {
        if ((uintptr_t)t->user & 2)
        {
            spi_give_buffer(
                (uint16_t *)t->tx_buffer);
        }

        xQueueSend(
            spi_transactions,
            &t,
            portMAX_DELAY);
    }
}

static void spi_init(void)
{
    spi_transactions = xQueueCreate(
        SPI_TRANSACTION_COUNT,
        sizeof(spi_transaction_t *));

    spi_buffers = xQueueCreate(
        SPI_BUFFER_COUNT,
        sizeof(uint16_t *));

    RG_ASSERT(
        spi_transactions && spi_buffers,
        "ST7735 queue allocation failed");

    while (uxQueueSpacesAvailable(spi_transactions))
    {
        void *trans = malloc(sizeof(spi_transaction_t));

        RG_ASSERT(
            trans != NULL,
            "ST7735 transaction allocation failed");

        xQueueSend(
            spi_transactions,
            &trans,
            portMAX_DELAY);
    }

    while (uxQueueSpacesAvailable(spi_buffers))
    {
        void *buffer = rg_alloc(
            SPI_BUFFER_LENGTH,
            MEM_DMA);

        RG_ASSERT(
            buffer != NULL,
            "ST7735 DMA allocation failed");

        xQueueSend(
            spi_buffers,
            &buffer,
            portMAX_DELAY);
    }

    const spi_bus_config_t buscfg = {
        .miso_io_num = RG_GPIO_LCD_MISO,
        .mosi_io_num = RG_GPIO_LCD_MOSI,
        .sclk_io_num = RG_GPIO_LCD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_BUFFER_LENGTH,
    };

    const spi_device_interface_config_t devcfg = {
        .clock_speed_hz = RG_SCREEN_SPEED,
        .mode = 0,
        .spics_io_num = RG_GPIO_LCD_CS,
        .queue_size = SPI_TRANSACTION_COUNT,
        .pre_cb = &spi_pre_transfer_cb,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    esp_err_t ret;

    ret = spi_bus_initialize(
        RG_SCREEN_HOST,
        &buscfg,
        SPI_DMA_CH_AUTO);

    RG_ASSERT(
        ret == ESP_OK || ret == ESP_ERR_INVALID_STATE,
        "ST7735 SPI bus initialization failed");

    ret = spi_bus_add_device(
        RG_SCREEN_HOST,
        &devcfg,
        &spi_dev);

    RG_ASSERT(
        ret == ESP_OK,
        "ST7735 SPI device initialization failed");

    rg_task_create(
        "rg_spi",
        &spi_task,
        NULL,
        1.5 * 1024,
        RG_TASK_PRIORITY_7,
        1);
}

static void spi_deinit(void)
{
    if (spi_bus_remove_device(spi_dev) == ESP_OK)
    {
        spi_bus_free(RG_SCREEN_HOST);
    }
    else
    {
        RG_LOGE(
            "Failed to terminate ST7735 SPI driver");
    }
}

// Backlight is connected directly to 3.3V.
// No PWM backlight GPIO is needed.
static void lcd_set_backlight(float percent)
{
    (void)percent;
}

// Set ST7735 drawing region
static void lcd_set_window(
    int left,
    int top,
    int width,
    int height)
{
    if (width <= 0 || height <= 0)
        return;

    int right = left + width - 1;
    int bottom = top + height - 1;

    if (left < 0 ||
        top < 0 ||
        right >= display.screen.real_width ||
        bottom >= display.screen.real_height)
    {
        RG_LOGW(
            "Bad ST7735 window: %d,%d to %d,%d",
            left, top, right, bottom);
    }

    int x0 = left + RG_ST7735_X_OFFSET;
    int y0 = top + RG_ST7735_Y_OFFSET;
    int x1 = right + RG_ST7735_X_OFFSET;
    int y1 = bottom + RG_ST7735_Y_OFFSET;

    // CASET - Column address
    ILI9341_CMD(
        0x2A,
        (x0 >> 8) & 0xFF,
        x0 & 0xFF,
        (x1 >> 8) & 0xFF,
        x1 & 0xFF);

    // RASET - Row address
    ILI9341_CMD(
        0x2B,
        (y0 >> 8) & 0xFF,
        y0 & 0xFF,
        (y1 >> 8) & 0xFF,
        y1 & 0xFF);

    // RAMWR - Memory write
    ILI9341_CMD(0x2C);
}

static inline uint16_t *lcd_get_buffer(size_t length)
{
    (void)length;
    return spi_take_buffer();
}

static inline void lcd_send_buffer(
    uint16_t *buffer,
    size_t length)
{
    if (length > 0)
    {
        spi_queue_transaction(
            buffer,
            length * sizeof(*buffer),
            3);
    }
    else
    {
        spi_give_buffer(buffer);
    }
}

static void lcd_sync(void)
{
    // SPI transaction queue handles transfers.
}

// ST7735 initialization
static void lcd_init(void)
{
    spi_init();

    // Data/command GPIO
    gpio_set_direction(
        RG_GPIO_LCD_DC,
        GPIO_MODE_OUTPUT);

    gpio_set_level(
        RG_GPIO_LCD_DC,
        1);

    // Hardware reset
#ifdef RG_GPIO_LCD_RST
    gpio_set_direction(
        RG_GPIO_LCD_RST,
        GPIO_MODE_OUTPUT);

    gpio_set_level(
        RG_GPIO_LCD_RST,
        0);

    rg_usleep(100000);

    gpio_set_level(
        RG_GPIO_LCD_RST,
        1);

    rg_usleep(120000);
#endif

    // SWRESET
    ILI9341_CMD(0x01);
    rg_usleep(150000);

    // SLPOUT
    ILI9341_CMD(0x11);
    rg_usleep(150000);

    // Frame rate control
    ILI9341_CMD(0xB1, 0x01, 0x2C, 0x2D);
    ILI9341_CMD(0xB2, 0x01, 0x2C, 0x2D);
    ILI9341_CMD(
        0xB3,
        0x01, 0x2C, 0x2D,
        0x01, 0x2C, 0x2D);

    // Display inversion control
    ILI9341_CMD(0xB4, 0x07);

    // Power control
    ILI9341_CMD(0xC0, 0xA2, 0x02, 0x84);
    ILI9341_CMD(0xC1, 0xC5);
    ILI9341_CMD(0xC2, 0x0A, 0x00);
    ILI9341_CMD(0xC3, 0x8A, 0x2A);
    ILI9341_CMD(0xC4, 0x8A, 0xEE);

    // VCOM control
    ILI9341_CMD(0xC5, 0x0E);

    // Positive gamma correction
    ILI9341_CMD(
        0xE0,
        0x02, 0x1C, 0x07, 0x12,
        0x37, 0x32, 0x29, 0x2D,
        0x29, 0x25, 0x2B, 0x39,
        0x00, 0x01, 0x03, 0x10);

    // Negative gamma correction
    ILI9341_CMD(
        0xE1,
        0x03, 0x1D, 0x07, 0x06,
        0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F,
        0x00, 0x00, 0x02, 0x10);

    // Color format: RGB565 (16-bit)
    ILI9341_CMD(0x3A, 0x05);

    // MADCTL: landscape, MX + MV + BGR
    ILI9341_CMD(0x36, 0x68);

    // Normal display mode
    ILI9341_CMD(0x13);
    rg_usleep(10000);

    // Display inversion OFF
    ILI9341_CMD(0x20);

    // Display ON
    ILI9341_CMD(0x29);
    rg_usleep(100000);
}

static void lcd_deinit(void)
{
#ifdef RG_SCREEN_DEINIT
    RG_SCREEN_DEINIT();
#endif

    spi_deinit();
}

const rg_display_driver_t rg_display_driver_st7735 = {
    .name = "st7735",
};
