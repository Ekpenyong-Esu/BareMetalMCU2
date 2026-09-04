/**
 * @file dma2d_core.h
 * @brief DMA2D lifecycle, layer configuration and driver state
 *
 * @details
 * Owns the single device record. The HAL handle is reached through
 * DMA2D_GetDevice() instead of an extern global, so the interrupt file, the
 * transfer module and the event module all share one definition.
 */

#ifndef DMA2D_CORE_H
#define DMA2D_CORE_H

#include "dma2d_types.h"

/**
 * @brief Access the single DMA2D device record
 * @return DMA2D_Device* The device record; never NULL
 */
DMA2D_Device *DMA2D_GetDevice(void);

/**
 * @brief Initialize the DMA2D peripheral with the specified configuration
 * @param config Pointer to DMA2D configuration structure (must not be NULL)
 * @return HAL_OK on success, HAL_ERROR on invalid parameters or HAL failure
 */
HAL_StatusTypeDef DMA2D_Init(const DMA2D_Config *config);

/**
 * @brief Deinitialize the DMA2D peripheral
 * @return HAL_OK on success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_DeInit(void);

/**
 * @brief Configure one DMA2D input layer
 * @param layer DMA2D_FOREGROUND_LAYER or DMA2D_BACKGROUND_LAYER
 * @param layer_config Pointer to layer configuration structure (must not be NULL)
 * @return HAL_OK on success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_ConfigLayer(uint32_t layer, const DMA2D_LayerConfig *layer_config);

/**
 * @brief Report whether the driver completed initialization
 * @return true when DMA2D_Init() succeeded and DMA2D_DeInit() has not run
 */
bool DMA2D_IsInitialized(void);

/**
 * @brief Check if a transfer is in progress
 * @return true if DMA2D is busy, false otherwise
 */
bool DMA2D_IsBusy(void);

/**
 * @brief Fold a HAL result into the driver counters
 * @param result HAL status of the operation that just finished
 */
void DMA2D_UpdateStatus(HAL_StatusTypeDef result);

/**
 * @brief Copy the driver status block
 * @param status Destination status structure (must not be NULL)
 * @return HAL_OK on success, HAL_ERROR when @p status is NULL
 */
HAL_StatusTypeDef DMA2D_GetStatus(DMA2D_Status *status);

/**
 * @brief Wait for the current transfer to finish
 * @param timeout Timeout in milliseconds
 * @return HAL status of the wait
 */
HAL_StatusTypeDef DMA2D_PollForTransfer(uint32_t timeout);

/**
 * @brief Abort the ongoing DMA2D transfer
 * @return HAL_OK on success, HAL status of the abort otherwise
 */
HAL_StatusTypeDef DMA2D_Abort(void);

/**
 * @brief Reconfigure the output stage for direct LCD framebuffer writes
 * @param color_mode One of DMA2D_OUTPUT_*, matching the LTDC layer's pixel
 *                  format; a mismatch here shows up as wrong colours, not as
 *                  an error.
 * @return HAL_OK on success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_EnableLCDMode(uint32_t color_mode);

/**
 * @brief DMA2D interrupt entry point
 * @details Core owns the vector table and calls this from DMA2D_IRQHandler().
 */
void DMA2D_ISR_Dispatch(void);

#endif /* DMA2D_CORE_H */
