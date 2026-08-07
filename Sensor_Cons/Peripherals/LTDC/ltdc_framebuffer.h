/**
 * @file ltdc_framebuffer.h
 * @brief Framebuffer ownership and direct pixel access
 */

#ifndef LTDC_FRAMEBUFFER_H
#define LTDC_FRAMEBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

/**
 * @brief Point a layer at a framebuffer and wait for the swap to take effect
 * @param driver Driver record
 * @param layer Layer number
 * @param address Framebuffer start address; must not be 0
 * @return HAL_StatusTypeDef HAL status, HAL_TIMEOUT if no reload event arrives
 */
HAL_StatusTypeDef LTDC_SetFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t address);

/**
 * @brief Fill a layer's framebuffer with a single colour
 * @param driver Driver record
 * @param layer Layer number
 * @param color Colour in the layer's own pixel format
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_ClearFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color);

/**
 * @brief Copy one layer's framebuffer into another
 * @param driver Driver record
 * @param srcLayer Source layer number
 * @param dstLayer Destination layer number
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_CopyFramebuffer(LTDC_Driver_t *driver, uint8_t srcLayer, uint8_t dstLayer);

/**
 * @brief Write one pixel to the active layer
 * @param driver Driver record
 * @param x X coordinate in panel space
 * @param y Y coordinate in panel space
 * @param color Colour in the layer's own pixel format
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_DrawPixel(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_FRAMEBUFFER_H */
