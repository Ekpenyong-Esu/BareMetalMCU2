/**
  ******************************************************************************
  * @file    ssd1306_buffer.c
  * @brief   SSD1306 frame buffer and screen refresh
  ******************************************************************************
  */

#include "ssd1306_buffer.h"
#include "ssd1306_cmd.h"
#include "ssd1306_io.h"
#include <string.h>

/* One shared panel, one frame buffer; the handle only carries cursor state. */
static uint8_t s_buffer[SSD1306_BUFFER_SIZE];

SSD1306_StatusTypeDef SSD1306_UpdateScreen(SSD1306_Handle_t *hssd)
{
    SSD1306_StatusTypeDef status;
    const uint8_t window[] = {
        SSD1306_CMD_COLUMN_ADDR, 0, SSD1306_WIDTH - 1,
        SSD1306_CMD_PAGE_ADDR,   0, SSD1306_PAGE_COUNT - 1
    };

    SSD1306_CHECK_HANDLE(hssd);

    status = SSD1306_IO_WriteCommands(&hssd->config, window, sizeof(window));
    if (status != SSD1306_OK) {
        return status;
    }

    return SSD1306_IO_WriteData(&hssd->config, s_buffer, SSD1306_BUFFER_SIZE);
}

SSD1306_StatusTypeDef SSD1306_Clear(SSD1306_Handle_t *hssd)
{
    return SSD1306_Fill(hssd, SSD1306_COLOR_BLACK);
}

SSD1306_StatusTypeDef SSD1306_Fill(SSD1306_Handle_t *hssd, SSD1306_Color_t color)
{
    if (hssd == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    memset(s_buffer, (color == SSD1306_COLOR_WHITE) ? 0xFF : 0x00, SSD1306_BUFFER_SIZE);
    hssd->currentX = 0;
    hssd->currentY = 0;

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_DrawPixel(SSD1306_Handle_t *hssd,
                                        uint16_t x, uint16_t y,
                                        SSD1306_Color_t color)
{
    if (hssd == NULL || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    uint16_t index = (uint16_t)(x + (y / 8U) * SSD1306_WIDTH);
    uint8_t mask = (uint8_t)(1U << (y % 8U));

    if (color == SSD1306_COLOR_WHITE) {
        s_buffer[index] |= mask;
    } else {
        s_buffer[index] &= (uint8_t)~mask;
    }

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_Buffer_SetColumn(uint16_t x, uint8_t page, uint8_t value)
{
    if (x >= SSD1306_WIDTH || page >= SSD1306_PAGE_COUNT) {
        return SSD1306_INVALID_PARAM;
    }

    s_buffer[x + (uint16_t)page * SSD1306_WIDTH] = value;

    return SSD1306_OK;
}
