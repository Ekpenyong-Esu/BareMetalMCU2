/**
 * @file dma_core.h
 * @brief DMA initialization and lifecycle
 */

#ifndef DMA_CORE_H
#define DMA_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dma_types.h"

/**
 * @brief   Initialize a DMA stream
 * @param   handle Handle to populate
 * @param   config Requested configuration
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DMA_Init(DMA_Handle_t *handle, const DMA_Config_t *config);

/**
 * @brief   Deinitialize a DMA stream and mask its interrupt
 * @param   handle Handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 * @note    The controller clock is left enabled because the other seven
 *          streams of the same controller may still be in use.
 */
HAL_StatusTypeDef DMA_DeInit(DMA_Handle_t *handle);

/**
 * @brief   Report whether the handle has been initialized
 * @param   handle Handle
 * @retval  bool true when initialized
 */
bool DMA_IsInitialized(const DMA_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* DMA_CORE_H */
