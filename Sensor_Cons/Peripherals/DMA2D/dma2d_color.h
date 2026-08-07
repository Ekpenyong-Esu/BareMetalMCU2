/**
 * @file dma2d_color.h
 * @brief ARGB8888 colour helpers and diagnostic strings for the DMA2D driver
 *
 * @details
 * Pure functions with no peripheral dependency: colour packing/unpacking and the
 * human-readable names used in log messages.
 */

#ifndef DMA2D_COLOR_H
#define DMA2D_COLOR_H

#include "dma2d_types.h"

/**
 * @brief Pack colour components into ARGB8888
 * @param red Red component (0-255)
 * @param green Green component (0-255)
 * @param blue Blue component (0-255)
 * @param alpha Alpha component (0-255)
 * @return uint32_t ARGB8888 color value
 */
uint32_t DMA2D_MakeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

/**
 * @brief Extract colour components from an ARGB8888 value
 * @param color ARGB8888 color value
 * @param red Destination for the red component (may be NULL)
 * @param green Destination for the green component (may be NULL)
 * @param blue Destination for the blue component (may be NULL)
 * @param alpha Destination for the alpha component (may be NULL)
 */
void DMA2D_GetColorComponents(uint32_t color, uint8_t *red, uint8_t *green,
                              uint8_t *blue, uint8_t *alpha);

/**
 * @brief Describe a HAL status code
 * @param error_code HAL error code
 * @return const char* Error description string
 */
const char* DMA2D_GetErrorString(HAL_StatusTypeDef error_code);

/**
 * @brief Describe a DMA2D peripheral state
 * @param state DMA2D state
 * @return const char* State description string
 */
const char* DMA2D_GetStateString(uint32_t state);

#endif /* DMA2D_COLOR_H */
