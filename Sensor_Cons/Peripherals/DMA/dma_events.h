/**
 * @file dma_events.h
 * @brief DMA interrupt dispatch and completion callbacks
 */

#ifndef DMA_EVENTS_H
#define DMA_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dma_types.h"

/**
 * @brief   Attach the driver dispatchers to a freshly initialized handle
 * @param   handle Handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DMA_Events_Register(DMA_Handle_t *handle);

/**
 * @brief   DMA stream interrupt entry point
 * @param   handle Handle owning the stream that raised the interrupt
 * @note    Call from the matching DMAx_StreamY_IRQHandler in stm32f4xx_it.c.
 */
void DMA_IRQHandler(DMA_Handle_t *handle);

/**
 * @brief   Transfer complete callback
 * @param   hdma HAL handle; hdma->Parent is the owning DMA_Handle_t
 */
void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma);

/**
 * @brief   Transfer error callback
 * @param   hdma HAL handle; hdma->Parent is the owning DMA_Handle_t
 */
void DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma);

#ifdef __cplusplus
}
#endif

#endif /* DMA_EVENTS_H */
