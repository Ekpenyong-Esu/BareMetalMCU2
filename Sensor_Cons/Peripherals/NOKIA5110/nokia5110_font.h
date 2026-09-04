/**
 ******************************************************************************
 * @file    nokia5110_font.h
 * @brief   5x7 ASCII font for the Nokia 5110 LCD
 ******************************************************************************
 */

#ifndef NOKIA5110_FONT_H
#define NOKIA5110_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nokia5110_types.h"

#define NOKIA5110_FONT_WIDTH 5U         /*!< Character width in pixels */
#define NOKIA5110_FONT_HEIGHT 7U        /*!< Character height in pixels */
#define NOKIA5110_FONT_SPACING 1U       /*!< Blank columns after a character */
#define NOKIA5110_FONT_FIRST_CHAR 0x20U /*!< First glyph in the table */
#define NOKIA5110_FONT_LAST_CHAR 0x7FU  /*!< Last glyph in the table */

/**
 * @brief  Column bitmap for a character.
 * @return NULL when @p character has no glyph, so callers must check.
 */
const uint8_t *NOKIA5110_FONT_GetGlyph(char character);

#ifdef __cplusplus
}
#endif

#endif /* NOKIA5110_FONT_H */
