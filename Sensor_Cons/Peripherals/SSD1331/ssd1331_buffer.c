/**
 ******************************************************************************
 * @file    ssd1331_buffer.c
 * @brief   SSD1331 frame buffer and screen refresh
 ******************************************************************************
 */

#include "ssd1331_buffer.h"
#include "ssd1331_cmd.h"
#include "ssd1331_io.h"

/* The handle's buffer is held in the panel's own byte order so a refresh is a
   straight SPI dump. */

/* RGB565 goes out big-endian: high byte first, then the low byte */
#define SSD1331_LOW_BYTE_MASK 0xFFU

static uint32_t SSD1331_Buffer_Offset(uint16_t posX, uint16_t posY) {
    return (((uint32_t)posY * SSD1331_WIDTH) + posX) * SSD1331_BYTES_PER_PIXEL;
}

void SSD1331_Buffer_SetPixel(SSD1331_Handle_t *hssd, uint16_t posX, uint16_t posY,
                             SSD1331_Color_t color) {
    uint32_t offset = SSD1331_Buffer_Offset(posX, posY);

    hssd->frameBuffer[offset] = (uint8_t)(color >> 8);
    hssd->frameBuffer[offset + 1U] = (uint8_t)(color & SSD1331_LOW_BYTE_MASK);
}

SSD1331_StatusTypeDef SSD1331_UpdateScreen(SSD1331_Handle_t *hssd) {
    const uint8_t window[] = {SSD1331_CMD_SET_COLUMN, 0U, SSD1331_WIDTH - 1U,
                              SSD1331_CMD_SET_ROW,    0U, SSD1331_HEIGHT - 1U};
    SSD1331_StatusTypeDef status = SSD1331_OK;

    SSD1331_CHECK_HANDLE(hssd);

    status = SSD1331_IO_WriteCommands(hssd, window, sizeof(window));
    if (status != SSD1331_OK) {
        return status;
    }

    return SSD1331_IO_WriteData(hssd, hssd->frameBuffer, SSD1331_BUFFER_SIZE);
}

SSD1331_StatusTypeDef SSD1331_Fill(SSD1331_Handle_t *hssd, SSD1331_Color_t color) {
    uint8_t high = (uint8_t)(color >> 8);
    uint8_t low = (uint8_t)(color & SSD1331_LOW_BYTE_MASK);

    SSD1331_CHECK_HANDLE(hssd);

    for (uint32_t offset = 0U; offset < SSD1331_BUFFER_SIZE; offset += SSD1331_BYTES_PER_PIXEL) {
        hssd->frameBuffer[offset] = high;
        hssd->frameBuffer[offset + 1U] = low;
    }

    hssd->currentX = 0U;
    hssd->currentY = 0U;

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_Clear(SSD1331_Handle_t *hssd) {
    return SSD1331_Fill(hssd, SSD1331_COLOR_BLACK);
}

SSD1331_StatusTypeDef SSD1331_DrawPixel(SSD1331_Handle_t *hssd, uint16_t posX, uint16_t posY,
                                        SSD1331_Color_t color) {
    SSD1331_CHECK_HANDLE(hssd);

    if (posX >= SSD1331_WIDTH || posY >= SSD1331_HEIGHT) {
        return SSD1331_INVALID_PARAM;
    }

    SSD1331_Buffer_SetPixel(hssd, posX, posY, color);

    return SSD1331_OK;
}
