/**
 ******************************************************************************
 * @file    ssd1331_text.c
 * @brief   SSD1331 text rendering
 ******************************************************************************
 */

#include "ssd1331_text.h"
#include "ssd1331_buffer.h"
#include "ssd1331_font.h"

SSD1331_StatusTypeDef SSD1331_SetCursor(SSD1331_Handle_t *hssd, uint16_t posX, uint16_t posY) {
    SSD1331_CHECK_HANDLE(hssd);

    if (posX >= SSD1331_WIDTH || posY >= SSD1331_HEIGHT) {
        return SSD1331_INVALID_PARAM;
    }

    hssd->currentX = posX;
    hssd->currentY = posY;

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_WriteChar(SSD1331_Handle_t *hssd, char chr, SSD1331_Color_t color,
                                        SSD1331_Color_t background) {
    const uint8_t *glyph = NULL;
    uint16_t posX = 0;
    uint16_t posY = 0;

    SSD1331_CHECK_HANDLE(hssd);

    glyph = SSD1331_FONT_GetGlyph(chr);
    if (glyph == NULL) {
        return SSD1331_INVALID_PARAM;
    }

    /* Work on copies so a rejected glyph leaves the cursor where it was. */
    posX = hssd->currentX;
    posY = hssd->currentY;

    if (posX + SSD1331_FONT_WIDTH > SSD1331_WIDTH) {
        posX = 0;
        posY = (uint16_t)(posY + SSD1331_FONT_HEIGHT);
    }

    if (posY + SSD1331_FONT_HEIGHT > SSD1331_HEIGHT) {
        return SSD1331_INVALID_PARAM;
    }

    for (uint8_t column = 0; column < SSD1331_FONT_WIDTH; column++) {
        uint8_t bits = glyph[column];

        for (uint8_t row = 0; row < SSD1331_FONT_HEIGHT; row++) {
            SSD1331_Color_t pixel = ((bits >> row) & 0x01U) ? color : background;
            SSD1331_Buffer_SetPixel(hssd, (uint16_t)(posX + column), (uint16_t)(posY + row), pixel);
        }
    }

    hssd->currentX = (uint16_t)(posX + SSD1331_FONT_WIDTH);
    hssd->currentY = posY;

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_WriteString(SSD1331_Handle_t *hssd, const char *str,
                                          SSD1331_Color_t color, SSD1331_Color_t background) {
    SSD1331_StatusTypeDef status = SSD1331_OK;

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
