/**
  ******************************************************************************
  * @file    ili9488_text.h
  * @brief   Cursor tracking and character rendering for the ILI9488
  ******************************************************************************
  */

#ifndef ILI9488_TEXT_H
#define ILI9488_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9488_types.h"

/**
 * @brief   Move the text cursor.
 * @retval  ILI9488_INVALID_PARAM if the position is outside the panel.
 */
ILI9488_StatusTypeDef ILI9488_SetCursor(ILI9488_Handle_t *hili, uint16_t x, uint16_t y);

/**
 * @brief   Render one character at the cursor and advance it.
 * @details Wraps to the next line when the glyph would run off the right edge.
 * @retval  ILI9488_INVALID_PARAM if @p ch has no glyph or the cursor has run
 *          past the bottom of the panel.
 */
ILI9488_StatusTypeDef ILI9488_WriteChar(ILI9488_Handle_t *hili, char ch,
                                        uint16_t color, uint16_t bgcolor);

#ifdef __cplusplus
}
#endif

#endif /* ILI9488_TEXT_H */
