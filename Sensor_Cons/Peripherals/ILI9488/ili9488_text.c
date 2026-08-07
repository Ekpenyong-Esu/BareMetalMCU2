/**
  ******************************************************************************
  * @file    ili9488_text.c
  * @brief   Cursor tracking and character rendering for the ILI9488
  ******************************************************************************
  */

#include "ili9488_text.h"
#include "ili9488_draw.h"
#include "ili9488_font.h"
#include "ili9488_io.h"
#include "ili9488_cmd.h"
#include <stddef.h>

#define ILI9488_GLYPH_PIXELS    (ILI9488_FONT_WIDTH * ILI9488_FONT_HEIGHT)

/**
 * @brief  Paint only the lit pixels, leaving whatever is already on screen
 *         between them. Selected by passing bgcolor equal to color.
 */
static ILI9488_StatusTypeDef ILI9488_DrawGlyphTransparent(ILI9488_Handle_t *hili,
                                                          const uint8_t *glyph,
                                                          uint16_t color)
{
    for (uint16_t col = 0U; col < ILI9488_FONT_WIDTH; col++) {
        for (uint16_t row = 0U; row < ILI9488_FONT_HEIGHT; row++) {
            if ((glyph[col] & (1U << row)) == 0U) {
                continue;
            }

            ILI9488_StatusTypeDef status = ILI9488_DrawPixel(hili,
                                                             (uint16_t)(hili->currentX + col),
                                                             (uint16_t)(hili->currentY + row),
                                                             color);
            if (status != ILI9488_OK) {
                return status;
            }
        }
    }

    return ILI9488_OK;
}

/**
 * @brief  Stream the whole 6x8 cell in one memory write, which costs a single
 *         address-window setup instead of one per pixel.
 */
static ILI9488_StatusTypeDef ILI9488_DrawGlyphOpaque(ILI9488_Handle_t *hili,
                                                     const uint8_t *glyph,
                                                     uint16_t color, uint16_t bgcolor)
{
    uint16_t cell[ILI9488_GLYPH_PIXELS];
    ILI9488_StatusTypeDef status;

    /* Row-major, matching the panel's scan order inside the window. */
    for (uint16_t row = 0U; row < ILI9488_FONT_HEIGHT; row++) {
        for (uint16_t col = 0U; col < ILI9488_FONT_WIDTH; col++) {
            bool lit = (glyph[col] & (1U << row)) != 0U;
            cell[(row * ILI9488_FONT_WIDTH) + col] = lit ? color : bgcolor;
        }
    }

    status = ILI9488_IO_SetAddressWindow(&hili->config,
                                         hili->currentX, hili->currentY,
                                         (uint16_t)(hili->currentX + ILI9488_FONT_WIDTH - 1U),
                                         (uint16_t)(hili->currentY + ILI9488_FONT_HEIGHT - 1U));
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_IO_WriteCommand(&hili->config, ILI9488_CMD_MEMORY_WRITE);
    if (status != ILI9488_OK) {
        return status;
    }

    return ILI9488_IO_WritePixelBuffer(&hili->config, cell, ILI9488_GLYPH_PIXELS);
}

ILI9488_StatusTypeDef ILI9488_SetCursor(ILI9488_Handle_t *hili, uint16_t x, uint16_t y)
{
    ILI9488_CHECK_HANDLE(hili);

    if (x >= hili->width || y >= hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    hili->currentX = x;
    hili->currentY = y;

    return ILI9488_OK;
}

ILI9488_StatusTypeDef ILI9488_WriteChar(ILI9488_Handle_t *hili, char ch,
                                        uint16_t color, uint16_t bgcolor)
{
    const uint8_t *glyph;
    ILI9488_StatusTypeDef status;
    uint16_t x = 0U;
    uint16_t y = 0U;

    ILI9488_CHECK_HANDLE(hili);

    glyph = ILI9488_FONT_GetGlyph(ch);
    if (glyph == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    /* Work out the placement first so a rejected character leaves the cursor
       exactly where it was. */
    x = hili->currentX;
    y = hili->currentY;

    if ((uint32_t)x + ILI9488_FONT_WIDTH > hili->width) {
        x = 0U;
        y = (uint16_t)(y + ILI9488_FONT_HEIGHT);
    }

    if ((uint32_t)y + ILI9488_FONT_HEIGHT > hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    hili->currentX = x;
    hili->currentY = y;

    status = (bgcolor == color)
                 ? ILI9488_DrawGlyphTransparent(hili, glyph, color)
                 : ILI9488_DrawGlyphOpaque(hili, glyph, color, bgcolor);

    if (status != ILI9488_OK) {
        return status;
    }

    hili->currentX = (uint16_t)(x + ILI9488_FONT_WIDTH);

    return ILI9488_OK;
}
