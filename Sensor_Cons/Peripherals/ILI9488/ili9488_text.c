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

#define ILI9488_GLYPH_PIXELS (ILI9488_FONT_WIDTH * ILI9488_FONT_HEIGHT)

/**
 * @brief  Paint only the lit pixels, leaving whatever is already on screen
 *         between them. Selected by passing bgcolor equal to color.
 */
static ILI9488_StatusTypeDef ILI9488_DrawGlyphTransparent(ILI9488_Handle_t *hili,
                                                          const uint8_t *glyph, uint16_t color) {
    for (uint16_t col = 0U; col < ILI9488_FONT_WIDTH; col++) {
        for (uint16_t row = 0U; row < ILI9488_FONT_HEIGHT; row++) {
            if ((glyph[col] & (1U << row)) == 0U) {
                continue;
            }

            ILI9488_StatusTypeDef status = ILI9488_DrawPixel(
                hili, (uint16_t)(hili->currentX + col), (uint16_t)(hili->currentY + row), color);
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
static ILI9488_StatusTypeDef ILI9488_DrawGlyphOpaque(ILI9488_Handle_t *hili, const uint8_t *glyph,
                                                     uint16_t color, uint16_t bgcolor) {
    uint16_t cell[ILI9488_GLYPH_PIXELS];
    ILI9488_StatusTypeDef status = ILI9488_OK;

    /* Row-major, matching the panel's scan order inside the window. */
    for (uint16_t row = 0U; row < ILI9488_FONT_HEIGHT; row++) {
        for (uint16_t col = 0U; col < ILI9488_FONT_WIDTH; col++) {
            bool lit = (glyph[col] & (1U << row)) != 0U;
            cell[(row * ILI9488_FONT_WIDTH) + col] = lit ? color : bgcolor;
        }
    }

    status = ILI9488_IO_SetAddressWindow(hili, hili->currentX, hili->currentY,
                                         (uint16_t)(hili->currentX + ILI9488_FONT_WIDTH - 1U),
                                         (uint16_t)(hili->currentY + ILI9488_FONT_HEIGHT - 1U));
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_IO_WriteCommand(hili, ILI9488_CMD_MEMORY_WRITE);
    if (status != ILI9488_OK) {
        return status;
    }

    return ILI9488_IO_WritePixelBuffer(hili, cell, ILI9488_GLYPH_PIXELS);
}

ILI9488_StatusTypeDef ILI9488_SetCursor(ILI9488_Handle_t *hili, uint16_t posX, uint16_t posY) {
    ILI9488_CHECK_HANDLE(hili);

    if (posX >= hili->width || posY >= hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    hili->currentX = posX;
    hili->currentY = posY;

    return ILI9488_OK;
}

ILI9488_StatusTypeDef ILI9488_WriteChar(ILI9488_Handle_t *hili, char chr, uint16_t color,
                                        uint16_t bgcolor) {
    const uint8_t *glyph = NULL;
    ILI9488_StatusTypeDef status = ILI9488_OK;
    uint16_t posX = 0U;
    uint16_t posY = 0U;

    ILI9488_CHECK_HANDLE(hili);

    glyph = ILI9488_FONT_GetGlyph(chr);
    if (glyph == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    /* Work out the placement first so a rejected character leaves the cursor
       exactly where it was. */
    posX = hili->currentX;
    posY = hili->currentY;

    if ((uint32_t)posX + ILI9488_FONT_WIDTH > hili->width) {
        posX = 0U;
        posY = (uint16_t)(posY + ILI9488_FONT_HEIGHT);
    }

    if ((uint32_t)posY + ILI9488_FONT_HEIGHT > hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    hili->currentX = posX;
    hili->currentY = posY;

    status = (bgcolor == color) ? ILI9488_DrawGlyphTransparent(hili, glyph, color)
                                : ILI9488_DrawGlyphOpaque(hili, glyph, color, bgcolor);

    if (status != ILI9488_OK) {
        return status;
    }

    hili->currentX = (uint16_t)(posX + ILI9488_FONT_WIDTH);

    return ILI9488_OK;
}
