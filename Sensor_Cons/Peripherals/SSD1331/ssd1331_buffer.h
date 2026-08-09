/**
  ******************************************************************************
  * @file    ssd1331_buffer.h
  * @brief   SSD1331 frame buffer and screen refresh
  ******************************************************************************
  */

#ifndef SSD1331_BUFFER_H
#define SSD1331_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1331_types.h"

/**
 * @brief   Push the frame buffer to the panel.
 */
SSD1331_StatusTypeDef SSD1331_UpdateScreen(SSD1331_Handle_t *hssd);

/**
 * @brief   Blank the frame buffer and home the text cursor.
 */
SSD1331_StatusTypeDef SSD1331_Clear(SSD1331_Handle_t *hssd);

/**
 * @brief   Flood the frame buffer with one colour and home the text cursor.
 */
SSD1331_StatusTypeDef SSD1331_Fill(SSD1331_Handle_t *hssd, SSD1331_Color_t color);

SSD1331_StatusTypeDef SSD1331_DrawPixel(SSD1331_Handle_t *hssd,
                                        uint16_t x, uint16_t y,
                                        SSD1331_Color_t color);

/**
 * @brief   Write one pixel without a handle check.
 * @note    Used by the text layer, which has already validated the handle and
 *          the bounds; not part of ssd1331.h.
 */
void SSD1331_Buffer_SetPixel(uint16_t x, uint16_t y, SSD1331_Color_t color);

#ifdef __cplusplus
}
#endif

#endif /* SSD1331_BUFFER_H */
