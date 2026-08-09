/**
  ******************************************************************************
  * @file    ssd1331_text.c
  * @brief   SSD1331 text rendering
  ******************************************************************************
  */

#include "ssd1331_text.h"
#include "ssd1331_buffer.h"
#include "ssd1331_font.h"

SSD1331_StatusTypeDef SSD1331_SetCursor(SSD1331_Handle_t *hssd, uint16_t x, uint16_t y)
{
    SSD1331_CHECK_HANDLE(hssd);

    if (x >= SSD1331_WIDTH || y >= SSD1331_HEIGHT) {
        return SSD1331_INVALID_PARAM;
    }

    hssd->currentX = x;
    hssd->currentY = y;

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_WriteChar(SSD1331_Handle_t *hssd, char ch,
                                        SSD1331_Color_t color, SSD1331_Color_t background)
{
    const uint8_t *glyph;
    uint16_t x;
    uint16_t y;

    SSD1331_CHECK_HANDLE(hssd);

    glyph = SSD1331_FONT_GetGlyph(ch);
    if (glyph == NULL) {
        return SSD1331_INVALID_PARAM;
    }

    /* Work on copies so a rejected glyph leaves the cursor where it was. */
    x = hssd->currentX;
    y = hssd->currentY;

    if (x + SSD1331_FONT_WIDTH > SSD1331_WIDTH) {
        x = 0;
        y = (uint16_t)(y + SSD1331_FONT_HEIGHT);
    }

    if (y + SSD1331_FONT_HEIGHT > SSD1331_HEIGHT) {
        return SSD1331_INVALID_PARAM;
    }

    for (uint8_t column = 0; column < SSD1331_FONT_WIDTH; column++) {
        uint8_t bits = glyph[column];

        for (uint8_t row = 0; row < SSD1331_FONT_HEIGHT; row++) {
            SSD1331_Color_t pixel = ((bits >> row) & 0x01U) ? color : background;
            SSD1331_Buffer_SetPixel((uint16_t)(x + column), (uint16_t)(y + row), pixel);
        }
    }

    hssd->currentX = (uint16_t)(x + SSD1331_FONT_WIDTH);
    hssd->currentY = y;

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_WriteString(SSD1331_Handle_t *hssd, const char *str,
                                          SSD1331_Color_t color, SSD1331_Color_t background)
{
    SSD1331_StatusTypeDef status;

    SSD1331_CHECK_HANDLE(hssd);

    if (str == NULL) {
        return SSD1331_INVALID_PARAM;
    }

    while (*str != '\0') {
        status = SSD1331_WriteChar(hssd, *str, color, background);
        if (status != SSD1331_OK) {
            return status;
        }
        str++;
    }

    return SSD1331_OK;
}
