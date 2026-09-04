/**
 * @file uart_interrupt.h
 * @brief Interrupt-driven transfer mode
 *
 * Transfers are started here and completed by the callbacks in uart_events.c.
 * Independently callable, like tim_ic.h: no vtable, no dispatch.
 *
 * Characteristics:
 * - Non-blocking: CPU can do other work while transfer progresses
 * - TX: HAL_UART_Transmit_IT() - one byte per TXE interrupt
 * - RX: HAL_UARTEx_ReceiveToIdle_IT() - fills landing buffer, IDLE interrupt
 *        marks frame end, data copied to ring buffer
 * - Ring buffer decouples ISR reception from application reads
 * - IDLE line detection enables variable-length frames without protocol
 * - Error interrupt (ERR) catches overrun, framing, noise, parity errors
 *
 * Data Flow (RX):
 *   1. UART_Interrupt_Init() enables IDLE + ERR interrupts
 *   2. UART_Interrupt_Receive() arms reception into rxBuffer (landing buffer)
 *   3. On IDLE: HAL copies rxBuffer -> rxRing, sets rxComplete flag
 *   4. Application calls UART_Interrupt_Receive() to drain rxRing
 *   5. If more data arrives before next Receive(), it queues in rxRing
 *
 * Use Cases:
 * - Moderate bandwidth with CPU multitasking
 * - Variable-length frames (IDLE detection)
 * - When DMA streams are scarce
 *
 * Usage Pattern:
 *   1. UART_Interrupt_Init() with config (mode = UART_MODE_INTERRUPT)
 *   2. UART_Interrupt_Transmit() to send (optionally wait with timeout)
 *   3. UART_Interrupt_Receive() to receive (drains ring buffer)
 *   4. UART_DeInit() to close
 */

#ifndef UART_INTERRUPT_H
#define UART_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a UART link in interrupt mode
 *
 * Configures the UART peripheral, initializes the ring buffer, enables
 * IDLE and ERR interrupts, and arms the first reception into the landing
 * buffer. If the handle was already open, it is deinitialized first.
 *
 * @param  handle UART handle to populate (must have huart, rxBuffer, rxSize set)
 * @param  config Desired configuration (baud rate, word length, stop bits, parity)
 * @retval UART_OK on success, UART_ERROR on invalid args or HAL init failure
 */
UART_Status_t UART_Interrupt_Init(UART_Handle_t *handle, const UART_Config_t *config);

/**
 * @brief  Start a send and optionally wait for the TX interrupt to finish it
 *
 * Calls HAL_UART_Transmit_IT() which enables TXE interrupt. Bytes are sent
 * one per interrupt. If timeout > 0, spins on txComplete flag until done.
 * If timeout == 0, returns immediately (non-blocking); caller must poll
 * txComplete or use callback.
 *
 * @param  handle  UART handle (must be initialized in interrupt mode)
 * @param  data    Bytes to send
 * @param  size    Number of bytes
 * @param  timeout Milliseconds to wait, 0 to return immediately
 * @retval UART_OK on success (or started if timeout==0), UART_ERROR on invalid args,
 *         UART_TIMEOUT_ERROR if timeout expired
 */
UART_Status_t UART_Interrupt_Transmit(UART_Handle_t *handle, const uint8_t *data, uint16_t size,
                                      uint32_t timeout);

/**
 * @brief  Start reception and wait for a full packet to drain from the ring
 *
 * If the ring buffer already has @p size bytes, returns immediately.
 * Otherwise, waits up to @p timeout for rxComplete flag (set by IDLE callback).
 * If timeout == 0, does a single non-blocking drain attempt.
 *
 * @param  handle  UART handle (must be initialized in interrupt mode)
 * @param  data    Destination buffer
 * @param  size    Number of bytes requested
 * @param  timeout Milliseconds to wait, 0 for a single non-blocking drain
 * @retval UART_OK on success, UART_TIMEOUT_ERROR if timeout expired,
 *         UART_ERROR on invalid args or ring buffer error
 */
UART_Status_t UART_Interrupt_Receive(UART_Handle_t *handle, uint8_t *data, uint16_t size,
                                     uint32_t timeout);

/**
 * @brief  Re-arm one-shot reception after a completed transfer
 *
 * Called by uart_events.c from HAL_UARTEx_RxEventCallback() after copying
 * the landing buffer to the ring buffer. Not part of the app-facing API.
 *
 * @param  handle UART handle
 * @retval true if re-arm succeeded, false on HAL error
 */
bool UART_Interrupt_Rearm(UART_Handle_t *handle);

/**
 * @brief  Rebuild the peripheral after a line error
 *
 * Called by uart_events.c from HAL_UART_ErrorCallback() on overrun/framing/
 * noise/parity error. Clears error flags and re-arms reception.
 * Not part of the app-facing API.
 *
 * @param  handle UART handle
 * @retval true if recovery succeeded, false on HAL error
 */
bool UART_Interrupt_Recover(UART_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_INTERRUPT_H */
