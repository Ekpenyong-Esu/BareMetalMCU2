/**
 * @file dma_stream.h
 * @brief Stream identification, clock gating and interrupt line ownership
 */

#ifndef DMA_STREAM_H
#define DMA_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dma_types.h"

/**
 * @brief   Resolve the interrupt line that serves a stream
 * @param   stream Stream instance
 * @param   irqn Destination for the interrupt number
 * @retval  bool true when the stream is one of the sixteen real streams
 * @note    Returns a status rather than a sentinel IRQn_Type, because every
 *          negative IRQn_Type value is a live system exception; -1 is
 *          SysTick_IRQn, so a sentinel would silently reprogram SysTick.
 */
bool DMA_GetStreamIRQ(const DMA_Stream_TypeDef *stream, IRQn_Type *irqn);

/**
 * @brief   Enable the controller clock and arm the stream interrupt
 * @param   handle Handle whose config.stream has already been populated
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DMA_EnableClockAndIRQ(DMA_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* DMA_STREAM_H */
