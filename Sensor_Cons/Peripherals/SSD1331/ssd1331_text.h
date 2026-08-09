/**
  ******************************************************************************
  * @file    ssd1331_text.h
  * @brief   SSD1331 text rendering
  ******************************************************************************
  */

#ifndef SSD1331_TEXT_H
#define SSD1331_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1331_types.h"

/**
 * @brief   Place the text cursor.
 */
SSD1331_StatusTypeDef SSD1331_SetCursor(SSD1331_Handle_t *hssd, uint16_t x, uint16_t y);

/**
 * @brief   Render one glyph in @p color over @p background.
 * @note    Wraps to the next line when the glyph does not fit on this one; the
 *          cursor is left untouched if the glyph does not fit on the screen.
 *          The background is painted too, so redrawing over existing text does
 *          not leave the old strokes behind.
 */
SSD1331_StatusTypeDef SSD1331_WriteChar(SSD1331_Handle_t *hssd, char ch,
                                        SSD1331_Color_t color, SSD1331_Color_t background);

SSD1331_StatusTypeDef SSD1331_WriteString(SSD1331_Handle_t *hssd, const char *str,
                                          SSD1331_Color_t color, SSD1331_Color_t background);

#ifdef __cplusplus
}
#endif

#endif /* SSD1331_TEXT_H */
