/**
  ******************************************************************************
  * @file    ili9488_io.c
  * @brief   SPI and control-line transport for the ILI9488
  ******************************************************************************
  */

#include "ili9488_io.h"
#include "ili9488_cmd.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"

/** The serial interface carries 3 bytes per pixel; RGB565 is parallel-only. */
#define ILI9488_BYTES_PER_PIXEL     3U

/** Pixels staged per SPI burst, bounding the stack cost of a fill */
#define ILI9488_FILL_CHUNK_PIXELS   64U

#define ILI9488_ADDR_PARAM_SIZE     4U

#define ILI9488_RESET_LOW_MS        100U
#define ILI9488_RESET_HIGH_MS       120U

#define ILI9488_RGB565_RED_MASK     0xF800U
#define ILI9488_RGB565_GREEN_MASK   0x07E0U
#define ILI9488_RGB565_BLUE_MASK    0x001FU

/* This panel's slot on the shared bus. */
static SPI_Device_t s_device;

static void ILI9488_IO_Select(const ILI9488_Config_t *config, GPIO_PinState dcState)
{
    HAL_GPIO_WritePin(config->dc_port, config->dc_pin, dcState);
    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_RESET);
}

static ILI9488_StatusTypeDef ILI9488_IO_Release(const ILI9488_Config_t *config,
                                                SPI_StatusTypeDef status)
{
    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_SET);

    if (status == SPI_TIMEOUT) {
        return ILI9488_TIMEOUT;
    }

    return (status == SPI_OK) ? ILI9488_OK : ILI9488_ERROR;
}

/** Widen RGB565 to the panel's 18-bit format, replicating the high bits. */
static void ILI9488_IO_ExpandColor(uint16_t color, uint8_t *bytes)
{
    uint8_t red = (uint8_t)((color & ILI9488_RGB565_RED_MASK) >> 8);
    uint8_t green = (uint8_t)((color & ILI9488_RGB565_GREEN_MASK) >> 3);
    uint8_t blue = (uint8_t)((color & ILI9488_RGB565_BLUE_MASK) << 3);

    bytes[0] = (uint8_t)(red | (red >> 5));
    bytes[1] = (uint8_t)(green | (green >> 6));
    bytes[2] = (uint8_t)(blue | (blue >> 5));
}

ILI9488_StatusTypeDef ILI9488_IO_Init(const ILI9488_Config_t *config)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (config == NULL || config->cs_port == NULL ||
        config->dc_port == NULL || config->rst_port == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = config->cs_pin;
    GPIO_Driver_Pin_Init(config->cs_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = config->dc_pin;
    GPIO_Driver_Pin_Init(config->dc_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(config->dc_port, config->dc_pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = config->rst_pin;
    GPIO_Driver_Pin_Init(config->rst_port, &GPIO_InitStruct);

    const SPI_ConfigTypeDef spiConfig = SPI_ConfigDefault();
    if (SPI_DeviceInit(&s_device, &spiConfig) != SPI_OK) {
        log_error("ILI9488: SPI device registration failed");
        return ILI9488_ERROR;
    }

    HAL_GPIO_WritePin(config->rst_port, config->rst_pin, GPIO_PIN_RESET);
    HAL_Delay(ILI9488_RESET_LOW_MS);
    HAL_GPIO_WritePin(config->rst_port, config->rst_pin, GPIO_PIN_SET);
    HAL_Delay(ILI9488_RESET_HIGH_MS);

    return ILI9488_OK;
}

ILI9488_StatusTypeDef ILI9488_IO_WriteCommand(const ILI9488_Config_t *config, uint8_t command)
{
    SPI_StatusTypeDef status;

    ILI9488_IO_Select(config, GPIO_PIN_RESET);
    status = SPI_Transmit(&s_device, &command, 1U, SPI_TIMEOUT_SHORT);

    return ILI9488_IO_Release(config, status);
}

ILI9488_StatusTypeDef ILI9488_IO_WriteData(const ILI9488_Config_t *config,
                                           const uint8_t *data, uint16_t size)
{
    SPI_StatusTypeDef status;

    if (data == NULL || size == 0U) {
        return ILI9488_INVALID_PARAM;
    }

    ILI9488_IO_Select(config, GPIO_PIN_SET);
    status = SPI_Transmit(&s_device, (uint8_t *)(uintptr_t)data, size, SPI_TIMEOUT_LONG);

    return ILI9488_IO_Release(config, status);
}

ILI9488_StatusTypeDef ILI9488_IO_WritePixels(const ILI9488_Config_t *config,
                                             uint16_t color, uint32_t count)
{
    uint8_t buffer[ILI9488_FILL_CHUNK_PIXELS * ILI9488_BYTES_PER_PIXEL];
    uint8_t pixel[ILI9488_BYTES_PER_PIXEL];
    SPI_StatusTypeDef status = SPI_OK;
    uint32_t remaining = count;

    if (count == 0U) {
        return ILI9488_INVALID_PARAM;
    }

    ILI9488_IO_ExpandColor(color, pixel);

    for (uint32_t i = 0U; i < ILI9488_FILL_CHUNK_PIXELS; i++) {
        buffer[(i * ILI9488_BYTES_PER_PIXEL) + 0U] = pixel[0];
        buffer[(i * ILI9488_BYTES_PER_PIXEL) + 1U] = pixel[1];
        buffer[(i * ILI9488_BYTES_PER_PIXEL) + 2U] = pixel[2];
    }

    /* Hold CS low across the whole run: the panel treats a rising CS as the
       end of the memory write. */
    ILI9488_IO_Select(config, GPIO_PIN_SET);

    while (remaining > 0U && status == SPI_OK) {
        uint32_t chunk = (remaining > ILI9488_FILL_CHUNK_PIXELS)
                             ? ILI9488_FILL_CHUNK_PIXELS : remaining;

        status = SPI_Transmit(&s_device, buffer, (uint16_t)(chunk * ILI9488_BYTES_PER_PIXEL),
                              SPI_TIMEOUT_LONG);
        remaining -= chunk;
    }

    return ILI9488_IO_Release(config, status);
}

ILI9488_StatusTypeDef ILI9488_IO_WritePixelBuffer(const ILI9488_Config_t *config,
                                                  const uint16_t *colors, uint32_t count)
{
    uint8_t buffer[ILI9488_FILL_CHUNK_PIXELS * ILI9488_BYTES_PER_PIXEL];
    SPI_StatusTypeDef status = SPI_OK;
    uint32_t sent = 0U;

    if (colors == NULL || count == 0U) {
        return ILI9488_INVALID_PARAM;
    }

    ILI9488_IO_Select(config, GPIO_PIN_SET);

    while (sent < count && status == SPI_OK) {
        uint32_t chunk = count - sent;

        if (chunk > ILI9488_FILL_CHUNK_PIXELS) {
            chunk = ILI9488_FILL_CHUNK_PIXELS;
        }

        for (uint32_t i = 0U; i < chunk; i++) {
            ILI9488_IO_ExpandColor(colors[sent + i], &buffer[i * ILI9488_BYTES_PER_PIXEL]);
        }

        status = SPI_Transmit(&s_device, buffer, (uint16_t)(chunk * ILI9488_BYTES_PER_PIXEL),
                              SPI_TIMEOUT_LONG);
        sent += chunk;
    }

    return ILI9488_IO_Release(config, status);
}

ILI9488_StatusTypeDef ILI9488_IO_SetAddressWindow(const ILI9488_Config_t *config,
                                                  uint16_t x0, uint16_t y0,
                                                  uint16_t x1, uint16_t y1)
{
    ILI9488_StatusTypeDef status;
    uint8_t params[ILI9488_ADDR_PARAM_SIZE];

    status = ILI9488_IO_WriteCommand(config, ILI9488_CMD_COLUMN_ADDR_SET);
    if (status != ILI9488_OK) {
        return status;
    }

    params[0] = (uint8_t)(x0 >> 8);
    params[1] = (uint8_t)(x0 & 0xFFU);
    params[2] = (uint8_t)(x1 >> 8);
    params[3] = (uint8_t)(x1 & 0xFFU);

    status = ILI9488_IO_WriteData(config, params, ILI9488_ADDR_PARAM_SIZE);
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_IO_WriteCommand(config, ILI9488_CMD_PAGE_ADDR_SET);
    if (status != ILI9488_OK) {
        return status;
    }

    params[0] = (uint8_t)(y0 >> 8);
    params[1] = (uint8_t)(y0 & 0xFFU);
    params[2] = (uint8_t)(y1 >> 8);
    params[3] = (uint8_t)(y1 & 0xFFU);

    return ILI9488_IO_WriteData(config, params, ILI9488_ADDR_PARAM_SIZE);
}
