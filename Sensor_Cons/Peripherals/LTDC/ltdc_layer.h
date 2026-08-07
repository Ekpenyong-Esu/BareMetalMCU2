/**
 * @file ltdc_layer.h
 * @brief LTDC layer configuration and geometry
 */

#ifndef LTDC_LAYER_H
#define LTDC_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

#define LTDC_MAX_ALPHA              255     /*!< Fully opaque layer alpha */

/**
 * @brief Apply a full layer configuration
 * @param driver Driver record
 * @param layer Layer number
 * @param config Layer configuration; RGB888 is rejected
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_ConfigureLayer(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *config);

/**
 * @brief Enable a layer and reload at the next VSYNC
 * @param driver Driver record
 * @param layer Layer number
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_EnableLayer(LTDC_Driver_t *driver, uint8_t layer);

/**
 * @brief Disable a layer immediately
 * @param driver Driver record
 * @param layer Layer number
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_DisableLayer(LTDC_Driver_t *driver, uint8_t layer);

/**
 * @brief Select the layer that drawing operations target
 * @param driver Driver record
 * @param layer Layer number
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_SetActiveLayer(LTDC_Driver_t *driver, uint8_t layer);

/**
 * @brief Set the constant alpha of a layer
 * @param driver Driver record
 * @param layer Layer number
 * @param alpha Alpha value (0-255)
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_SetLayerAlpha(LTDC_Driver_t *driver, uint8_t layer, uint8_t alpha);

/**
 * @brief Move a layer window, keeping its size, and reload at the next VSYNC
 * @param driver Driver record
 * @param layer Layer number
 * @param x New left position
 * @param y New top position
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_SetLayerPosition(LTDC_Driver_t *driver, uint8_t layer, uint16_t x, uint16_t y);

/**
 * @brief Move a layer window without requesting a reload
 * @param driver Driver record
 * @param x New left position
 * @param y New top position
 * @param layer Layer number
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_SetWindowPosition_NoReload(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint8_t layer);

/**
 * @brief Set both position and size of a layer window
 * @param driver Driver record
 * @param layer Layer number
 * @param window Window rectangle
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_SetLayerWindow(LTDC_Driver_t *driver, uint8_t layer, LTDC_Rect_t *window);

/**
 * @brief Copy out the cached configuration of a layer
 * @param driver Driver record
 * @param layer Layer number
 * @param info Destination
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_GetLayerInfo(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *info);

/**
 * @brief Report whether a layer is currently enabled
 * @param driver Driver record
 * @param layer Layer number
 * @return bool true when enabled
 */
bool LTDC_IsLayerEnabled(LTDC_Driver_t *driver, uint8_t layer);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_LAYER_H */
