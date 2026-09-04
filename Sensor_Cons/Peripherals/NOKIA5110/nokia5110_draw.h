/**
 ******************************************************************************
 * @file    nokia5110_draw.h
 * @brief   Framebuffer drawing primitives
 ******************************************************************************
 */

#ifndef NOKIA5110_DRAW_H
#define NOKIA5110_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nokia5110_types.h"

/**
 * @brief   Set or clear one pixel.
 * @retval  NOKIA5110_INVALID_PARAM if the coordinates are off screen.
 * @note    The primitives below clip instead of failing.
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawPixel(NOKIA5110_Handle_t *hnok, uint8_t posX, uint8_t posY,
                                            uint8_t color);

/**
 * @brief   Draw a line with Bresenham's algorithm, clipped to the panel.
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawLine(NOKIA5110_Handle_t *hnok, int16_t startX, int16_t startY,
                                           int16_t endX, int16_t endY, uint8_t color);

/**
 * @brief   Draw a rectangle outline.
 * @retval  NOKIA5110_INVALID_PARAM if @p width or @p height is zero.
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawRect(NOKIA5110_Handle_t *hnok, int16_t posX, int16_t posY,
                                           uint8_t width, uint8_t height, uint8_t color);

/**
 * @brief   Draw a filled rectangle.
 * @retval  NOKIA5110_INVALID_PARAM if @p width or @p height is zero.
 */
NOKIA5110_StatusTypeDef NOKIA5110_FillRect(NOKIA5110_Handle_t *hnok, int16_t posX, int16_t posY,
                                           uint8_t width, uint8_t height, uint8_t color);

/**
 * @brief   Draw a circle outline with Bresenham's algorithm.
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawCircle(NOKIA5110_Handle_t *hnok, int16_t centerX,
                                             int16_t centerY, uint8_t radius, uint8_t color);

/**
 * @brief   Draw a string in the 5x7 font.
 * @param   posX    Pixel column of the first glyph
 * @param   textRow Text row, 0-5. '\\n' returns to @p posX on the next row.
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawText(NOKIA5110_Handle_t *hnok, uint8_t posX, uint8_t textRow,
                                           const char *text, uint8_t color);

#ifdef __cplusplus
}
#endif

#endif /* NOKIA5110_DRAW_H */
