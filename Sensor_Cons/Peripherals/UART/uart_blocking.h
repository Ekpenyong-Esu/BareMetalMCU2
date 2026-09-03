/**
 * @file uart_blocking.h
 * @brief Blocking transfer mode (polling)
 *
 * The CPU waits inside each transfer call until the transfer completes or
 * times out. Nothing runs in interrupt context, so this mode is a plain,
 * independently-callable API: no vtable, no dispatch, no callbacks.
 *
 * Characteristics:
 * - Simplest to use and debug
 * - CPU is blocked during transfer (not suitable for high throughput)
 * - No interrupts enabled (RXNE, TC, IDLE, ERR all disabled)
 * - Uses HAL_UART_Transmit() and HAL_UARTEx_ReceiveToIdle()
 * - ReceiveToIdle returns on line idle, so variable-length frames work naturally
 *
 * Use Cases:
 * - Low-bandwidth command/response protocols
 * - Bootloaders, debug consoles, simple sensors
 * - Situations where interrupts are undesirable
 *
 * Usage Pattern:
 *   1. UART_Blocking_Init() with config (mode = UART_MODE_BLOCKING)
 *   2. UART_Blocking_Transmit() to send
 *   3. UART_Blocking_Receive() to receive (returns on idle line)
 *   4. UART_DeInit() to close
 */

#ifndef UART_BLOCKING_H
#define UART_BLOCKING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a UART link in blocking mode
 *
 * Configures the UART peripheral with the given settings, disables all
 * interrupt sources (RXNE, TC, PE, ERR, IDLE), and marks the handle as
 * initialized. If the handle was already open, it is deinitialized first.
 *
 * @param  handle UART handle to populate (must have huart, rxBuffer, rxSize set)
 * @param  config Desired configuration (baud rate, word length, stop bits, parity)
 * @retval UART_OK on success, UART_ERROR on invalid args or HAL init failure
 */
UART_Status_t UART_Blocking_Init(UART_Handle_t *handle, const UART_Config_t *config);

/**
 * @brief  Send data, waiting for it to leave the peripheral
 *
 * Calls HAL_UART_Transmit() which polls the TXE/TC flags until all bytes
 * are shifted out or timeout expires. The CPU does not return until done.
 *
 * @param  handle  UART handle (must be initialized in blocking mode)
 * @param  data    Bytes to send
 * @param  size    Number of bytes
 * @param  timeout Milliseconds to wait for transfer to complete
 * @retval UART_OK on success, UART_ERROR on invalid args, UART_TIMEOUT_ERROR on timeout
 */
UART_Status_t UART_Blocking_Transmit(UART_Handle_t *handle, const uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Receive data, waiting for it to arrive
 *
 * Uses HAL_UARTEx_ReceiveToIdle() which returns when either:
 * - The requested number of bytes is received, OR
 * - The line goes idle (no data for ~1 character time)
 *
 * This means a frame shorter than @p size is a normal success. The actual
 * byte count is written to @p received. Bytes past @p received are left
 * untouched; the caller must not assume a terminator.
 *
 * On timeout with some data received, returns UART_OK (partial frame).
 * On timeout with no data, returns UART_TIMEOUT_ERROR.
 * On overrun/framing/noise error, clears ORE flag and returns UART_ERROR.
 *
 * @param  handle   UART handle (must be initialized in blocking mode)
 * @param  data     Destination buffer
 * @param  size     Capacity of @p data in bytes
 * @param  received Out: bytes actually written (set even on failure)
 * @param  timeout  Milliseconds to wait
 * @retval UART_OK on success (including partial frame), UART_TIMEOUT_ERROR if nothing arrived,
 *         UART_ERROR on overrun/framing/noise error or invalid args
 */
UART_Status_t UART_Blocking_Receive(UART_Handle_t *handle, uint8_t *data, uint16_t size,
                                    uint16_t *received, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* UART_BLOCKING_H */
