/**
 * @file ltdc_core.h
 * @brief LTDC driver lifecycle, display configuration and shared validation
 * @details Owns the HAL handle and the checks every other LTDC module performs
 *          before touching the peripheral.
 */

#ifndef LTDC_CORE_H
#define LTDC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

/**
 * @brief Access the HAL handle of the single LTDC controller
 * @return LTDC_HandleTypeDef* The handle; never NULL
 */
LTDC_HandleTypeDef* LTDC_GetHandle(void);

/**
 * @brief LTDC interrupt entry point
 * @details Core owns the vector table and calls this from LTDC_IRQHandler().
 */
void LTDC_ISR_Dispatch(void);

/* Lifecycle -----------------------------------------------------------------*/

/**
 * @brief Initialize an LTDC driver record with panel defaults
 * @param driver Driver record to fill in
 * @param hltdc_handle HAL handle the driver should drive
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_Driver_Init(LTDC_Driver_t *driver, LTDC_HandleTypeDef *hltdc_handle);

/**
 * @brief Disable all layers and release the controller
 * @param driver Driver record
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_Driver_DeInit(LTDC_Driver_t *driver);

/**
 * @brief Program the panel timings and polarities
 * @param driver Driver record
 * @param config Display configuration
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_ConfigureDisplay(LTDC_Driver_t *driver, LTDC_DisplayConfig_t *config);

/* Display control -----------------------------------------------------------*/

/**
 * @brief Set the colour shown where no layer is visible
 * @param driver Driver record
 * @param color Background colour, 0x00RRGGBB
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_SetBackgroundColor(LTDC_Driver_t *driver, uint32_t color);

/**
 * @brief Enable the LTDC controller
 * @param driver Driver record
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_DisplayOn(LTDC_Driver_t *driver);

/**
 * @brief Disable the LTDC controller
 * @param driver Driver record
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_DisplayOff(LTDC_Driver_t *driver);

/* Error handling ------------------------------------------------------------*/

/**
 * @brief Read the last recorded error code
 * @param driver Driver record
 * @return uint32_t One of the LTDC_ERROR_* codes
 */
uint32_t LTDC_GetError(LTDC_Driver_t *driver);

/**
 * @brief Clear the recorded error code
 * @param driver Driver record
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_ClearError(LTDC_Driver_t *driver);

/* Shared validation ---------------------------------------------------------*/

/**
 * @brief Check that a driver record is usable
 * @param driver Driver record
 * @return HAL_OK when the record is initialised and has a handle
 */
HAL_StatusTypeDef LTDC_ValidateDriver(LTDC_Driver_t *driver);

/**
 * @brief Check that a layer index exists
 * @param layer Layer number
 * @return HAL_OK when the layer is within LTDC_MAX_LAYERS
 */
HAL_StatusTypeDef LTDC_ValidateLayer(uint8_t layer);

/**
 * @brief Check that a coordinate lies on the panel
 * @param xCoord X coordinate
 * @param yCoord Y coordinate
 * @return HAL_OK when the point is on screen
 */
HAL_StatusTypeDef LTDC_ValidateCoordinates(uint16_t xCoord, uint16_t yCoord);

/**
 * @brief Check that a rectangle fits on the panel
 * @param rect Rectangle to test
 * @return HAL_OK when the rectangle is fully on screen
 */
HAL_StatusTypeDef LTDC_ValidateRect(LTDC_Rect_t *rect);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_CORE_H */
