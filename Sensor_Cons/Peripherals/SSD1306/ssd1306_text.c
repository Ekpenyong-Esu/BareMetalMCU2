/**
 ******************************************************************************
 * @file    ssd1306_text.c
 * @brief   SSD1306 text rendering
 ******************************************************************************
 */

#include "ssd1306_text.h"
#include "ssd1306_buffer.h"
#include "ssd1306_font.h"

SSD1306_StatusTypeDef SSD1306_SetCursor(SSD1306_Handle_t *hssd, uint16_t posX, uint16_t posY) {
    SSD1306_CHECK_HANDLE(hssd);

    if (posX >= SSD1306_WIDTH || posY >= SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    hssd->currentX = posX;
    /* A glyph spans exactly one page, so the row must sit on a page boundary. */
    hssd->currentY = (uint16_t)(posY & ~(uint16_t)(SSD1306_FONT_HEIGHT - 1U));

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_WriteChar(SSD1306_Handle_t *hssd, char chr, SSD1306_Color_t color) {
    const uint8_t *glyph = NULL;
    uint16_t posX = 0;
    uint16_t posY = 0;

    SSD1306_CHECK_HANDLE(hssd);

    glyph = SSD1306_FONT_GetGlyph(chr);
    if (glyph == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    /* Work on copies so a rejected glyph leaves the cursor where it was. */
    posX = hssd->currentX;
    posY = hssd->currentY;

    if (posX + SSD1306_FONT_WIDTH > SSD1306_WIDTH) {
        posX = 0;
        posY = (uint16_t)(posY + SSD1306_FONT_HEIGHT);
    }

    if (posY + SSD1306_FONT_HEIGHT > SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < SSD1306_FONT_WIDTH; i++) {
        /* Writing the whole column also paints the background, so redrawing
           over existing text does not leave the old strokes behind. */
        uint8_t column = (color == SSD1306_COLOR_WHITE) ? glyph[i] : (uint8_t)~glyph[i];
        SSD1306_StatusTypeDef status = SSD1306_Buffer_SetColumn(
            hssd, (uint16_t)(posX + i), (uint8_t)(posY / SSD1306_FONT_HEIGHT), column);
        if (status != SSD1306_OK) {
            return status;
        }
    }

    hssd->currentX = (uint16_t)(posX + SSD1306_FONT_WIDTH);
    hssd->currentY = posY;

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_WriteString(SSD1306_Handle_t *hssd, const char *str,
                                          SSD1306_Color_t color) {
    SSD1306_StatusTypeDef status = SSD1306_OK;

    SSD1306_CHECK_HANDLE(hssd);

    if (str == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    while (*str != '\0') {
        status = SSD1306_WriteChar(hssd, *str, color);
        if (status != SSD1306_OK) {
            return status;
        }
        str++;
    }

    return SSD1306_OK;
}
