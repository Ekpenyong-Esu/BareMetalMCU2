/**
 * @file uart_dma.h
 * @brief DMA transfer mode
 *
 * Buffer transfers are offloaded to a DMA stream; the streams themselves are
 * wired up by HAL_UART_MspInit() in Core. Independently callable, like
 * tim_ic.h: no vtable, no dispatch.
 */

#ifndef UART_DMA_H
#define UART_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a link in DMA mode
 * @note   Re-initializes the handle if it is already open. Fails if no DMA
 *         stream has been linked to the HAL handle (see HAL_UART_MspInit()).
 * @param  handle UART handle to populate
 * @param  config Desired configuration
 * @retval UART_OK on success
 */
UART_Status_t UART_DMA_Init(UART_Handle_t *handle, const UART_Config_t *config);

/**
 * @brief  Start a send and optionally wait for the DMA transfer to finish it
 * @param  handle  UART handle
 * @param  data    Bytes to send
 * @param  size    Number of bytes
 * @param  timeout Milliseconds to wait, 0 to return immediately
 * @retval UART_OK on success
 */
UART_Status_t UART_DMA_Transmit(UART_Handle_t *handle, const uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Start reception and wait for a full packet to drain from the ring
 * @param  handle  UART handle
 * @param  data    Destination buffer
 * @param  size    Number of bytes requested
 * @param  timeout Milliseconds to wait, 0 for a single non-blocking drain
 * @retval UART_OK on success
 */
UART_Status_t UART_DMA_Receive(UART_Handle_t *handle, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Re-arm one-shot reception after a completed transfer
 * @details Called by uart_events.c; not part of the app-facing API.
 */
void UART_DMA_Rearm(UART_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_DMA_H */
