/**
  ******************************************************************************
  * @file    ssd1306_text.c
  * @brief   SSD1306 text rendering
  ******************************************************************************
  */

#include "ssd1306_text.h"
#include "ssd1306_buffer.h"
#include "ssd1306_font.h"

SSD1306_StatusTypeDef SSD1306_SetCursor(SSD1306_Handle_t *hssd, uint16_t x, uint16_t y)
{
    SSD1306_CHECK_HANDLE(hssd);

    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    hssd->currentX = x;
    /* A glyph spans exactly one page, so the row must sit on a page boundary. */
    hssd->currentY = (uint16_t)(y & ~(uint16_t)(SSD1306_FONT_HEIGHT - 1U));

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_WriteChar(SSD1306_Handle_t *hssd, char ch, SSD1306_Color_t color)
{
    const uint8_t *glyph;
    uint16_t x;
    uint16_t y;

    SSD1306_CHECK_HANDLE(hssd);

    glyph = SSD1306_FONT_GetGlyph(ch);
    if (glyph == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    /* Work on copies so a rejected glyph leaves the cursor where it was. */
    x = hssd->currentX;
    y = hssd->currentY;

    if (x + SSD1306_FONT_WIDTH > SSD1306_WIDTH) {
        x = 0;
        y = (uint16_t)(y + SSD1306_FONT_HEIGHT);
    }

    if (y + SSD1306_FONT_HEIGHT > SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < SSD1306_FONT_WIDTH; i++) {
        /* Writing the whole column also paints the background, so redrawing
           over existing text does not leave the old strokes behind. */
        uint8_t column = (color == SSD1306_COLOR_WHITE) ? glyph[i] : (uint8_t)~glyph[i];
        SSD1306_StatusTypeDef status =
            SSD1306_Buffer_SetColumn((uint16_t)(x + i),
                                     (uint8_t)(y / SSD1306_FONT_HEIGHT), column);
        if (status != SSD1306_OK) {
            return status;
        }
    }

    hssd->currentX = (uint16_t)(x + SSD1306_FONT_WIDTH);
    hssd->currentY = y;

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_WriteString(SSD1306_Handle_t *hssd, const char *str,
                                          SSD1306_Color_t color)
{
    SSD1306_StatusTypeDef status;

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
