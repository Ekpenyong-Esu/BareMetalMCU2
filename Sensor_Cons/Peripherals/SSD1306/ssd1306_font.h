/**
  ******************************************************************************
  * @file    ssd1306_font.h
  * @brief   6x8 ASCII font for the SSD1306
  ******************************************************************************
  */

#ifndef SSD1306_FONT_H
#define SSD1306_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306_types.h"

#define SSD1306_FONT_FIRST_CHAR   32U
#define SSD1306_FONT_CHAR_COUNT   96U

/**
 * @brief   Column bytes for @p c, bit N of byte i being row N of column i.
 * @retval  NULL if the font has no glyph for that character.
 */
const uint8_t *SSD1306_FONT_GetGlyph(char c);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_FONT_H */
