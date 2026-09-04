/**
 ******************************************************************************
 * @file    nokia5110_draw.c
 * @brief   Framebuffer drawing primitives
 ******************************************************************************
 */

#include "nokia5110_draw.h"
#include "nokia5110_font.h"
#include <stdlib.h>

/** Write a pixel, discarding anything outside the panel. */
static void NOKIA5110_SetPixel(NOKIA5110_Handle_t *hnok, int16_t posX, int16_t posY,
                               uint8_t color) {
    uint8_t row = 0U;
    uint8_t bit = 0U;

    if (posX < 0 || posX >= NOKIA5110_WIDTH || posY < 0 || posY >= NOKIA5110_HEIGHT) {
        return;
    }

    row = (uint8_t)(posY / NOKIA5110_ROW_HEIGHT);
    bit = (uint8_t)(posY % NOKIA5110_ROW_HEIGHT);

    if (color != 0U) {
        hnok->Buffer[row][posX] |= (uint8_t)(1U << bit);
    }
    else {
        hnok->Buffer[row][posX] &= (uint8_t) ~(1U << bit);
    }
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawPixel(NOKIA5110_Handle_t *hnok, uint8_t posX, uint8_t posY,
                                            uint8_t color) {
    NOKIA5110_CHECK_HANDLE(hnok);

    if (posX >= NOKIA5110_WIDTH || posY >= NOKIA5110_HEIGHT) {
        return NOKIA5110_INVALID_PARAM;
    }

    NOKIA5110_SetPixel(hnok, (int16_t)posX, (int16_t)posY, color);

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawLine(NOKIA5110_Handle_t *hnok, int16_t startX, int16_t startY,
                                           int16_t endX, int16_t endY, uint8_t color) {
    /* The Bresenham terms are kept 32-bit: a difference of two int16_t
       coordinates does not fit an int16_t. */
    int32_t deltaX = 0;
    int32_t deltaY = 0;
    int32_t stepX = 0;
    int32_t stepY = 0;
    int32_t err = 0;

    NOKIA5110_CHECK_HANDLE(hnok);

    deltaX = abs(endX - startX);
    deltaY = abs(endY - startY);
    stepX = (startX < endX) ? 1 : -1;
    stepY = (startY < endY) ? 1 : -1;
    err = deltaX - deltaY;

    for (;;) {
        NOKIA5110_SetPixel(hnok, startX, startY, color);

        if (startX == endX && startY == endY) {
            break;
        }

        const int32_t doubleErr = 2 * err;

        /* The cursor only ever moves towards the end point, so it stays
           within the int16_t range of the coordinates. */
        if (doubleErr > -deltaY) {
            err -= deltaY;
            startX = (int16_t)(startX + stepX);
        }
        if (doubleErr < deltaX) {
            err += deltaX;
            startY = (int16_t)(startY + stepY);
        }
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawRect(NOKIA5110_Handle_t *hnok, int16_t posX, int16_t posY,
                                           uint8_t width, uint8_t height, uint8_t color) {
    int16_t right = 0;
    int16_t bottom = 0;

    NOKIA5110_CHECK_HANDLE(hnok);

    /* A zero extent would put the far edge one pixel before the near one. */
    if (width == 0U || height == 0U) {
        return NOKIA5110_INVALID_PARAM;
    }

    right = (int16_t)(posX + width - 1);
    bottom = (int16_t)(posY + height - 1);

    (void)NOKIA5110_DrawLine(hnok, posX, posY, right, posY, color);
    (void)NOKIA5110_DrawLine(hnok, posX, bottom, right, bottom, color);
    (void)NOKIA5110_DrawLine(hnok, posX, posY, posX, bottom, color);
    (void)NOKIA5110_DrawLine(hnok, right, posY, right, bottom, color);

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_FillRect(NOKIA5110_Handle_t *hnok, int16_t posX, int16_t posY,
                                           uint8_t width, uint8_t height, uint8_t color) {
    NOKIA5110_CHECK_HANDLE(hnok);

    if (width == 0U || height == 0U) {
        return NOKIA5110_INVALID_PARAM;
    }

    for (uint8_t i = 0U; i < height; i++) {
        (void)NOKIA5110_DrawLine(hnok, posX, (int16_t)(posY + i), (int16_t)(posX + width - 1),
                                 (int16_t)(posY + i), color);
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawCircle(NOKIA5110_Handle_t *hnok, int16_t centerX,
                                             int16_t centerY, uint8_t radius, uint8_t color) {
    int16_t offsetX = 0;
    int16_t offsetY = 0;
    int32_t err = 0;

    NOKIA5110_CHECK_HANDLE(hnok);

    offsetX = (int16_t)radius;

    while (offsetX >= offsetY) {
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX + offsetX), (int16_t)(centerY + offsetY), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX + offsetY), (int16_t)(centerY + offsetX), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX - offsetY), (int16_t)(centerY + offsetX), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX - offsetX), (int16_t)(centerY + offsetY), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX - offsetX), (int16_t)(centerY - offsetY), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX - offsetY), (int16_t)(centerY - offsetX), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX + offsetY), (int16_t)(centerY - offsetX), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(centerX + offsetX), (int16_t)(centerY - offsetY), color);

        offsetY += 1;
        err += 1 + 2 * offsetY;
        if ((2 * (err - offsetX) + 1) > 0) {
            offsetX -= 1;
            err += 1 - 2 * offsetX;
        }
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawText(NOKIA5110_Handle_t *hnok, uint8_t posX, uint8_t textRow,
                                           const char *text, uint8_t color) {
    int16_t cursorX = 0;
    int16_t cursorY = 0;

    NOKIA5110_CHECK_HANDLE(hnok);

    if (text == NULL || textRow >= NOKIA5110_ROWS) {
        return NOKIA5110_INVALID_PARAM;
    }

    cursorX = (int16_t)posX;
    cursorY = (int16_t)(textRow * NOKIA5110_ROW_HEIGHT);

    while (*text != '\0') {
        if (*text == '\n') {
            cursorX = (int16_t)posX;
            cursorY += NOKIA5110_ROW_HEIGHT;
        }
        else {
            const uint8_t *glyph = NOKIA5110_FONT_GetGlyph(*text);

            if (glyph != NULL) {
                for (uint8_t i = 0U; i < NOKIA5110_FONT_WIDTH; i++) {
                    uint8_t column = glyph[i];

                    for (uint8_t j = 0U; j < NOKIA5110_FONT_HEIGHT; j++) {
                        if ((column & (1U << j)) != 0U) {
                            NOKIA5110_SetPixel(hnok, (int16_t)(cursorX + i), (int16_t)(cursorY + j),
                                               color);
                        }
                    }
                }

                cursorX += (int16_t)(NOKIA5110_FONT_WIDTH + NOKIA5110_FONT_SPACING);
            }
        }

        text++;
    }

    return NOKIA5110_OK;
}
