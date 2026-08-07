/**
 * @file dma2d_validate.h
 * @brief Parameter validation for the DMA2D driver
 *
 * @details
 * Isolates every "is this value acceptable" decision. The checks are pure
 * functions of their arguments (plus the configured mode for transfers), so
 * they can be reasoned about without any peripheral state.
 */

#ifndef DMA2D_VALIDATE_H
#define DMA2D_VALIDATE_H

#include "dma2d_types.h"

/**
 * @brief Validate DMA2D configuration parameters
 * @param config Pointer to configuration structure
 * @return HAL_OK when the configuration is usable, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_ValidateConfig(const DMA2D_Config *config);

/**
 * @brief Validate DMA2D layer configuration parameters
 * @param layer_config Pointer to layer configuration structure
 * @return HAL_OK when the configuration is usable, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_ValidateLayerConfig(const DMA2D_LayerConfig *layer_config);

/**
 * @brief Validate a layer selector
 * @param layer DMA2D_FOREGROUND_LAYER or DMA2D_BACKGROUND_LAYER
 * @return HAL_OK when the selector names a real layer, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_ValidateLayer(uint32_t layer);

/**
 * @brief Validate transfer geometry and buffers
 * @param mode   Configured operating mode; decides whether a source is required
 * @param pSrc   Source buffer, may be NULL for register-to-memory transfers
 * @param pDst   Destination buffer
 * @param width  Transfer width in pixels
 * @param height Transfer height in pixels
 * @return HAL_OK when the transfer can be started, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_ValidateTransfer(uint32_t mode, const uint32_t *pSrc,
                                         const uint32_t *pDst,
                                         uint32_t width, uint32_t height);

#endif /* DMA2D_VALIDATE_H */
