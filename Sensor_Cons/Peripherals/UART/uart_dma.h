/**
 * @file uart_dma.h
 * @brief DMA transfer mode
 *
 * Buffer transfers are offloaded to a DMA stream; the streams themselves are
 * wired up by HAL_UART_MspInit() in Core. Independently callable, like
 * tim_ic.h: no vtable, no dispatch.
 *
 * Characteristics:
 * - Highest throughput: CPU completely free during transfer
 * - TX: HAL_UART_Transmit_DMA() - DMA moves buffer to DR register
 * - RX: HAL_UARTEx_ReceiveToIdle_DMA() - DMA fills landing buffer, IDLE
 *        triggers transfer complete, data copied to ring buffer
 * - Requires DMA streams configured in HAL_UART_MspInit() (Core)
 * - Ring buffer decouples DMA reception from application reads
 * - IDLE line detection enables variable-length frames
 *
 * Data Flow (RX):
 *   1. UART_DMA_Init() enables IDLE interrupt, configures DMA streams
 *   2. UART_DMA_Receive() arms DMA reception into rxBuffer
 *   3. On IDLE: DMA stops, HAL copies rxBuffer -> rxRing, sets rxComplete
 *   4. Application calls UART_DMA_Receive() to drain rxRing
 *   5. If more data arrives before next Receive(), it queues in rxRing
 *
 * Use Cases:
 * - High-bandwidth continuous streams (e.g., sensor data, file transfer)
 * - When CPU must not be blocked at all
 * - Large buffers (DMA handles arbitrary sizes)
 *
 * Usage Pattern:
 *   1. UART_DMA_Init() with config (mode = UART_MODE_DMA)
 *   2. UART_DMA_Transmit() to send (optionally wait with timeout)
 *   3. UART_DMA_Receive() to receive (drains ring buffer)
 *   4. UART_DeInit() to close
 */

#ifndef UART_DMA_H
#define UART_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a UART link in DMA mode
 *
 * Configures the UART peripheral, initializes the ring buffer, enables
 * IDLE interrupt, and verifies DMA streams are attached to the HAL handle.
 * If the handle was already open, it is deinitialized first.
 *
 * @note   Fails if no DMA stream has been linked to the HAL handle
 *         (see HAL_UART_MspInit() in Core/Src/stm32f4xx_hal_msp.c).
 *
 * @param  handle UART handle to populate (must have huart, rxBuffer, rxSize set)
 * @param  config Desired configuration (baud rate, word length, stop bits, parity)
 * @retval UART_OK on success, UART_ERROR on invalid args, missing DMA, or HAL init failure
 */
UART_Status_t UART_DMA_Init(UART_Handle_t *handle, const UART_Config_t *config);

/**
 * @brief  Start a send and optionally wait for the DMA transfer to finish it
 *
 * Calls HAL_UART_Transmit_DMA() which starts the DMA stream. If timeout > 0,
 * spins on txComplete flag (set by HAL_UART_TxCpltCallback) until done.
 * If timeout == 0, returns immediately (non-blocking).
 *
 * @param  handle  UART handle (must be initialized in DMA mode)
 * @param  data    Bytes to send
 * @param  size    Number of bytes
 * @param  timeout Milliseconds to wait, 0 to return immediately
 * @retval UART_OK on success (or started if timeout==0), UART_ERROR on invalid args,
 *         UART_TIMEOUT_ERROR if timeout expired
 */
UART_Status_t UART_DMA_Transmit(UART_Handle_t *handle, const uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Start reception and wait for a full packet to drain from the ring
 *
 * If the ring buffer already has @p size bytes, returns immediately.
 * Otherwise, waits up to @p timeout for rxComplete flag (set by IDLE callback).
 * If timeout == 0, does a single non-blocking drain attempt.
 *
 * @param  handle  UART handle (must be initialized in DMA mode)
 * @param  data    Destination buffer
 * @param  size    Number of bytes requested
 * @param  timeout Milliseconds to wait, 0 for a single non-blocking drain
 * @retval UART_OK on success, UART_TIMEOUT_ERROR if timeout expired,
 *         UART_ERROR on invalid args or ring buffer error
 */
UART_Status_t UART_DMA_Receive(UART_Handle_t *handle, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Re-arm one-shot reception after a completed transfer
 *
 * Called by uart_events.c from HAL_UARTEx_RxEventCallback() after copying
 * the landing buffer to the ring buffer. Not part of the app-facing API.
 *
 * @param  handle UART handle
 */
void UART_DMA_Rearm(UART_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_DMA_H */
