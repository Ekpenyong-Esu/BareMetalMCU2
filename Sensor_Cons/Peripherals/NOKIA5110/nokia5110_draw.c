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
static void NOKIA5110_SetPixel(NOKIA5110_Handle_t *hnok, int16_t x, int16_t y, uint8_t color)
{
    uint8_t row = 0U;
    uint8_t bit = 0U;

    if (x < 0 || x >= NOKIA5110_WIDTH || y < 0 || y >= NOKIA5110_HEIGHT) {
        return;
    }

    row = (uint8_t)(y / NOKIA5110_ROW_HEIGHT);
    bit = (uint8_t)(y % NOKIA5110_ROW_HEIGHT);

    if (color != 0U) {
        hnok->Buffer[row][x] |= (uint8_t)(1U << bit);
    } else {
        hnok->Buffer[row][x] &= (uint8_t)~(1U << bit);
    }
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawPixel(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y,
                                            uint8_t color)
{
    NOKIA5110_CHECK_HANDLE(hnok);

    if (x >= NOKIA5110_WIDTH || y >= NOKIA5110_HEIGHT) {
        return NOKIA5110_INVALID_PARAM;
    }

    NOKIA5110_SetPixel(hnok, (int16_t)x, (int16_t)y, color);

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawLine(NOKIA5110_Handle_t *hnok, int16_t x0, int16_t y0,
                                           int16_t x1, int16_t y1, uint8_t color)
{
    int16_t dx;
    int16_t dy;
    int16_t sx;
    int16_t sy;
    int16_t err;

    NOKIA5110_CHECK_HANDLE(hnok);

    dx = (int16_t)abs(x1 - x0);
    dy = (int16_t)abs(y1 - y0);
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    err = dx - dy;

    for (;;) {
        NOKIA5110_SetPixel(hnok, x0, y0, color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int16_t e2 = (int16_t)(2 * err);

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawRect(NOKIA5110_Handle_t *hnok, int16_t x, int16_t y,
                                           uint8_t width, uint8_t height, uint8_t color)
{
    int16_t right;
    int16_t bottom;

    NOKIA5110_CHECK_HANDLE(hnok);

    /* A zero extent would put the far edge one pixel before the near one. */
    if (width == 0U || height == 0U) {
        return NOKIA5110_INVALID_PARAM;
    }

    right = (int16_t)(x + width - 1);
    bottom = (int16_t)(y + height - 1);

    (void)NOKIA5110_DrawLine(hnok, x, y, right, y, color);
    (void)NOKIA5110_DrawLine(hnok, x, bottom, right, bottom, color);
    (void)NOKIA5110_DrawLine(hnok, x, y, x, bottom, color);
    (void)NOKIA5110_DrawLine(hnok, right, y, right, bottom, color);

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_FillRect(NOKIA5110_Handle_t *hnok, int16_t x, int16_t y,
                                           uint8_t width, uint8_t height, uint8_t color)
{
    NOKIA5110_CHECK_HANDLE(hnok);

    if (width == 0U || height == 0U) {
        return NOKIA5110_INVALID_PARAM;
    }

    for (uint8_t i = 0U; i < height; i++) {
        (void)NOKIA5110_DrawLine(hnok, x, (int16_t)(y + i), (int16_t)(x + width - 1),
                                 (int16_t)(y + i), color);
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawCircle(NOKIA5110_Handle_t *hnok, int16_t x0, int16_t y0,
                                             uint8_t radius, uint8_t color)
{
    int16_t x;
    int16_t y = 0;
    int16_t err = 0;

    NOKIA5110_CHECK_HANDLE(hnok);

    x = (int16_t)radius;

    while (x >= y) {
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 + x), (int16_t)(y0 + y), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 + y), (int16_t)(y0 + x), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 - y), (int16_t)(y0 + x), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 - x), (int16_t)(y0 + y), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 - x), (int16_t)(y0 - y), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 - y), (int16_t)(y0 - x), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 + y), (int16_t)(y0 - x), color);
        NOKIA5110_SetPixel(hnok, (int16_t)(x0 + x), (int16_t)(y0 - y), color);

        y += 1;
        err += 1 + 2 * y;
        if ((2 * (err - x) + 1) > 0) {
            x -= 1;
            err += 1 - 2 * x;
        }
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DrawText(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y,
                                           const char *text, uint8_t color)
{
    int16_t cursorX;
    int16_t cursorY;

    NOKIA5110_CHECK_HANDLE(hnok);

    if (text == NULL || y >= NOKIA5110_ROWS) {
        return NOKIA5110_INVALID_PARAM;
    }

    cursorX = (int16_t)x;
    cursorY = (int16_t)(y * NOKIA5110_ROW_HEIGHT);

    while (*text != '\0') {
        if (*text == '\n') {
            cursorX = (int16_t)x;
            cursorY += NOKIA5110_ROW_HEIGHT;
        } else {
            const uint8_t *glyph = NOKIA5110_FONT_GetGlyph(*text);

            if (glyph != NULL) {
                for (uint8_t i = 0U; i < NOKIA5110_FONT_WIDTH; i++) {
                    uint8_t column = glyph[i];

                    for (uint8_t j = 0U; j < NOKIA5110_FONT_HEIGHT; j++) {
                        if ((column & (1U << j)) != 0U) {
                            NOKIA5110_SetPixel(hnok, (int16_t)(cursorX + i),
                                               (int16_t)(cursorY + j), color);
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
