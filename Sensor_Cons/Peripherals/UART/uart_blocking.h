/**
 * @file uart_blocking.h
 * @brief Blocking transfer mode
 *
 * The CPU waits inside each transfer. Nothing runs in interrupt context, so
 * this mode is a plain, independently-callable API: no vtable, no dispatch.
 */

#ifndef UART_BLOCKING_H
#define UART_BLOCKING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a link in blocking mode
 * @note   Re-initializes the handle if it is already open.
 * @param  handle UART handle to populate
 * @param  config Desired configuration
 * @retval UART_OK on success
 */
UART_Status_t UART_Blocking_Init(UART_Handle_t *handle, const UART_Config_t *config);

/**
 * @brief  Send data, waiting for it to leave the peripheral
 * @param  handle  UART handle
 * @param  data    Bytes to send
 * @param  size    Number of bytes
 * @param  timeout Milliseconds to wait
 * @retval UART_OK on success
 */
UART_Status_t UART_Blocking_Transmit(UART_Handle_t *handle, const uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Receive data, waiting for it to arrive
 * @note   Returns as soon as the line goes idle, so a frame shorter than
 *         @p size is a normal success. Bytes past @p received are left
 *         untouched; the caller must not assume a terminator.
 * @param  handle   UART handle
 * @param  data     Destination buffer
 * @param  size     Capacity of @p data in bytes
 * @param  received Out: bytes actually written, set even on failure
 * @param  timeout  Milliseconds to wait
 * @retval UART_OK on success, UART_TIMEOUT_ERROR if nothing arrived
 */
UART_Status_t UART_Blocking_Receive(UART_Handle_t *handle, uint8_t *data, uint16_t size,
                                    uint16_t *received, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* UART_BLOCKING_H */
