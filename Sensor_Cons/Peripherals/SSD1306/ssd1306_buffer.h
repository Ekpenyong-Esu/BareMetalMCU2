/**
  ******************************************************************************
  * @file    ssd1306_buffer.h
  * @brief   SSD1306 frame buffer and screen refresh
  ******************************************************************************
  */

#ifndef SSD1306_BUFFER_H
#define SSD1306_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306_types.h"

/**
 * @brief   Push the frame buffer to the panel.
 */
SSD1306_StatusTypeDef SSD1306_UpdateScreen(SSD1306_Handle_t *hssd);

/**
 * @brief   Blank the frame buffer and home the text cursor.
 */
SSD1306_StatusTypeDef SSD1306_Clear(SSD1306_Handle_t *hssd);

/**
 * @brief   Set every pixel in the frame buffer and home the text cursor.
 */
SSD1306_StatusTypeDef SSD1306_Fill(SSD1306_Handle_t *hssd, SSD1306_Color_t color);

SSD1306_StatusTypeDef SSD1306_DrawPixel(SSD1306_Handle_t *hssd,
                                        uint16_t x, uint16_t y,
                                        SSD1306_Color_t color);

/**
 * @brief   Replace one whole 8-pixel column of a page.
 * @note    Used by the text layer to place a glyph column in a single store.
 */
SSD1306_StatusTypeDef SSD1306_Buffer_SetColumn(uint16_t x, uint8_t page, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_BUFFER_H */
