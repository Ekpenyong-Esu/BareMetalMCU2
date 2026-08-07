/**
  ******************************************************************************
  * @file    ili9488_font.h
  * @brief   6x8 ASCII font for the ILI9488 driver
  ******************************************************************************
  */

#ifndef ILI9488_FONT_H
#define ILI9488_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9488_types.h"

/**
 * @brief  Column bitmap for a character.
 * @return NULL when @p c has no glyph, so callers must check.
 */
const uint8_t *ILI9488_FONT_GetGlyph(char c);

#ifdef __cplusplus
}
#endif

#endif /* ILI9488_FONT_H */
