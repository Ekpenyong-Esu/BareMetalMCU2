/**
 * @file dma2d_events.h
 * @brief Application callbacks invoked from DMA2D interrupt context
 *
 * @details
 * The only public surface of the event module is "who gets told". The HAL
 * callback overrides themselves live in dma2d_events.c and must not be called
 * directly.
 */

#ifndef DMA2D_EVENTS_H
#define DMA2D_EVENTS_H

#include "dma2d_types.h"

/**
 * @brief Register the transfer complete callback
 * @param callback Function pointer to callback (NULL to disable)
 * @note Runs in interrupt context, keep it short
 */
void DMA2D_RegisterTransferCompleteCallback(DMA2D_TransferCompleteCallback callback);

/**
 * @brief Register the transfer error callback
 * @param callback Function pointer to callback (NULL to disable)
 * @note Runs in interrupt context, keep it short
 */
void DMA2D_RegisterTransferErrorCallback(DMA2D_TransferErrorCallback callback);

/**
 * @brief Register the transfer progress callback
 * @param callback Function pointer to callback (NULL to disable)
 * @note This feature may not be available on all STM32 series
 */
void DMA2D_RegisterTransferProgressCallback(DMA2D_TransferProgressCallback callback);

/**
 * @brief Forget every registered callback
 */
void DMA2D_ClearCallbacks(void);

#endif /* DMA2D_EVENTS_H */
