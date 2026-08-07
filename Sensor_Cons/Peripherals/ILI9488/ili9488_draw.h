/**
  ******************************************************************************
  * @file    ili9488_draw.h
  * @brief   Pixel-level drawing primitives for the ILI9488
  ******************************************************************************
  */

#ifndef ILI9488_DRAW_H
#define ILI9488_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9488_types.h"

/**
 * @brief   Draw a single pixel.
 * @retval  ILI9488_INVALID_PARAM if the coordinates are outside the panel.
 */
ILI9488_StatusTypeDef ILI9488_DrawPixel(ILI9488_Handle_t *hili,
                                        uint16_t x, uint16_t y, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* ILI9488_DRAW_H */
