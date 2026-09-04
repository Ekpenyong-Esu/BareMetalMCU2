/**
 ******************************************************************************
 * @file    ssd1306_text.h
 * @brief   SSD1306 text rendering
 ******************************************************************************
 */

#ifndef SSD1306_TEXT_H
#define SSD1306_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306_types.h"

/**
 * @brief   Place the text cursor; posY is rounded down to a page boundary.
 */
SSD1306_StatusTypeDef SSD1306_SetCursor(SSD1306_Handle_t *hssd, uint16_t posX, uint16_t posY);

/**
 * @brief   Render one glyph, painting its background as well as its strokes.
 * @note    Wraps to the next line when the glyph does not fit on this one; the
 *          cursor is left untouched if the glyph does not fit on the screen.
 */
SSD1306_StatusTypeDef SSD1306_WriteChar(SSD1306_Handle_t *hssd, char chr, SSD1306_Color_t color);

SSD1306_StatusTypeDef SSD1306_WriteString(SSD1306_Handle_t *hssd, const char *str,
                                          SSD1306_Color_t color);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_TEXT_H */
