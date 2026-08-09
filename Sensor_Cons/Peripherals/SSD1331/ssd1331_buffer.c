/**
  ******************************************************************************
  * @file    ssd1331_buffer.c
  * @brief   SSD1331 frame buffer and screen refresh
  ******************************************************************************
  */

#include "ssd1331_buffer.h"
#include "ssd1331_cmd.h"
#include "ssd1331_io.h"

/* Held in the panel's own byte order so a refresh is a straight SPI dump. */
static uint8_t s_frameBuffer[SSD1331_BUFFER_SIZE];

static uint32_t SSD1331_Buffer_Offset(uint16_t x, uint16_t y)
{
    return (((uint32_t)y * SSD1331_WIDTH) + x) * SSD1331_BYTES_PER_PIXEL;
}

void SSD1331_Buffer_SetPixel(uint16_t x, uint16_t y, SSD1331_Color_t color)
{
    uint32_t offset = SSD1331_Buffer_Offset(x, y);

    s_frameBuffer[offset] = (uint8_t)(color >> 8);
    s_frameBuffer[offset + 1U] = (uint8_t)(color & 0xFFU);
}

SSD1331_StatusTypeDef SSD1331_UpdateScreen(SSD1331_Handle_t *hssd)
{
    const uint8_t window[] = {
        SSD1331_CMD_SET_COLUMN, 0U, SSD1331_WIDTH - 1U,
        SSD1331_CMD_SET_ROW,    0U, SSD1331_HEIGHT - 1U
    };
    SSD1331_StatusTypeDef status = SSD1331_OK;

    SSD1331_CHECK_HANDLE(hssd);

    status = SSD1331_IO_WriteCommands(&hssd->config, window, sizeof(window));
    if (status != SSD1331_OK) {
        return status;
    }

    return SSD1331_IO_WriteData(&hssd->config, s_frameBuffer, SSD1331_BUFFER_SIZE);
}

SSD1331_StatusTypeDef SSD1331_Fill(SSD1331_Handle_t *hssd, SSD1331_Color_t color)
{
    uint8_t high = (uint8_t)(color >> 8);
    uint8_t low = (uint8_t)(color & 0xFFU);

    SSD1331_CHECK_HANDLE(hssd);

    for (uint32_t offset = 0U; offset < SSD1331_BUFFER_SIZE; offset += SSD1331_BYTES_PER_PIXEL) {
        s_frameBuffer[offset] = high;
        s_frameBuffer[offset + 1U] = low;
    }

    hssd->currentX = 0U;
    hssd->currentY = 0U;

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_Clear(SSD1331_Handle_t *hssd)
{
    return SSD1331_Fill(hssd, SSD1331_COLOR_BLACK);
}

SSD1331_StatusTypeDef SSD1331_DrawPixel(SSD1331_Handle_t *hssd,
                                        uint16_t x, uint16_t y,
                                        SSD1331_Color_t color)
{
    SSD1331_CHECK_HANDLE(hssd);

    if (x >= SSD1331_WIDTH || y >= SSD1331_HEIGHT) {
        return SSD1331_INVALID_PARAM;
    }

    SSD1331_Buffer_SetPixel(x, y, color);

    return SSD1331_OK;
}
