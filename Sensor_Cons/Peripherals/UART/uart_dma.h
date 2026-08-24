/**
 * @file uart_dma.h
 * @brief DMA transfer mode
 *
 * Same shape as uart_interrupt.h on purpose: Init() arms reception and it
 * stays armed, Read() takes what has piled up, Write() returns while the send
 * is still going out. Nothing waits.
 *
 * The difference is underneath. Interrupt mode takes one interrupt per byte
 * and the CPU copies each one; DMA hands the whole buffer to a stream and the
 * CPU is only interrupted when the transfer ends. Identical to use, much
 * cheaper at high baud rates.
 *
 * The streams themselves are wired up by HAL_UART_MspInit() in Core.
 * Independently callable, like tim_ic.h: no vtable, no dispatch.
 */

#ifndef UART_DMA_H
#define UART_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a link in DMA mode and start listening
 * @note   Re-initializes the handle if it is already open. Fails if no DMA
 *         stream has been linked to the HAL handle (see HAL_UART_MspInit()).
 * @param  rxBuffer     Landing area the stream writes into; must outlive the
 *                      handle, and must sit in DMA-reachable RAM (on the
 *                      F429 that means ordinary SRAM, never CCMRAM)
 * @param  rxBufferSize Size of @p rxBuffer, at most RING_BUFFER_SIZE
 * @retval UART_OK on success
 */
UART_Status_t UART_DMA_Init(UART_Handle_t *handle, const UART_Config_t *config,
                            uint8_t *rxBuffer, uint16_t rxBufferSize);

/**
 * @brief  Take whatever has been received so far, up to @p size bytes
 * @note   Returns immediately. A quiet line is not an error: @p received is
 *         set to 0 and the status is still UART_OK. Bytes past @p received
 *         are untouched, so never treat @p data as a string.
 * @param  received Out: bytes actually written, set even on failure
 * @retval UART_OK unless the arguments were wrong
 */
UART_Status_t UART_DMA_Read(UART_Handle_t *handle, uint8_t *data, uint16_t size,
                            uint16_t *received);

/**
 * @brief  Start sending and return; the DMA stream does the work
 * @note   @p data belongs to the stream until UART_DMA_IsTxDone() goes true.
 *         Reusing or freeing it before then corrupts the transmission, and it
 *         must live in DMA-reachable RAM for the same reason as the RX buffer.
 * @retval UART_BUSY if the previous send is still in flight
 */
UART_Status_t UART_DMA_Write(UART_Handle_t *handle, const uint8_t *data, uint16_t size);

/**
 * @brief  Whether the last Write() has finished leaving the peripheral
 * @note   True when idle, so it is safe to call before the first Write().
 */
bool UART_DMA_IsTxDone(const UART_Handle_t *handle);

/**
 * @brief  Re-arm reception after a completed transfer
 * @details Called by uart_events.c; not part of the app-facing API.
 */
void UART_DMA_Rearm(UART_Handle_t *handle);

/**
 * @brief  Release a transfer the error path tore down
 * @details Called by uart_events.c; not part of the app-facing API.
 */
void UART_DMA_Recover(UART_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_DMA_H */
