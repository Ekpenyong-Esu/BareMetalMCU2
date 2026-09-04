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

/* One page byte covers 8 vertical pixels; all set lights the whole column */
#define SSD1306_PAGE_ALL_ON 0xFF
#define SSD1306_PAGE_ALL_OFF 0x00

SSD1306_StatusTypeDef SSD1306_UpdateScreen(SSD1306_Handle_t *hssd) {
    SSD1306_StatusTypeDef status = SSD1306_OK;
    const uint8_t window[] = {SSD1306_CMD_COLUMN_ADDR, 0, SSD1306_WIDTH - 1,
                              SSD1306_CMD_PAGE_ADDR,   0, SSD1306_PAGE_COUNT - 1};

    SSD1306_CHECK_HANDLE(hssd);

    status = SSD1306_IO_WriteCommands(hssd, window, sizeof(window));
    if (status != SSD1306_OK) {
        return status;
    }

    return SSD1306_IO_WriteData(hssd, hssd->buffer, SSD1306_BUFFER_SIZE);
}

SSD1306_StatusTypeDef SSD1306_Clear(SSD1306_Handle_t *hssd) {
    return SSD1306_Fill(hssd, SSD1306_COLOR_BLACK);
}

SSD1306_StatusTypeDef SSD1306_Fill(SSD1306_Handle_t *hssd, SSD1306_Color_t color) {
    if (hssd == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    memset(hssd->buffer,
           (color == SSD1306_COLOR_WHITE) ? SSD1306_PAGE_ALL_ON : SSD1306_PAGE_ALL_OFF,
           SSD1306_BUFFER_SIZE);
    hssd->currentX = 0;
    hssd->currentY = 0;

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_DrawPixel(SSD1306_Handle_t *hssd, uint16_t posX, uint16_t posY,
                                        SSD1306_Color_t color) {
    if (hssd == NULL || posX >= SSD1306_WIDTH || posY >= SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    uint16_t index = (uint16_t)(posX + (posY / 8U) * SSD1306_WIDTH);
    uint8_t mask = (uint8_t)(1U << (posY % 8U));

    if (color == SSD1306_COLOR_WHITE) {
        hssd->buffer[index] |= mask;
    }
    else {
        hssd->buffer[index] &= (uint8_t)~mask;
    }

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_Buffer_SetColumn(SSD1306_Handle_t *hssd, uint16_t posX, uint8_t page,
                                               uint8_t value) {
    if (hssd == NULL || posX >= SSD1306_WIDTH || page >= SSD1306_PAGE_COUNT) {
        return SSD1306_INVALID_PARAM;
    }

    hssd->buffer[posX + (uint16_t)page * SSD1306_WIDTH] = value;

    return SSD1306_OK;
}
