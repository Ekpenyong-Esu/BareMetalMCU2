/**
 * @file uart_interrupt.h
 * @brief Interrupt-driven transfer mode
 *
 * Transfers are started here and completed by the callbacks in uart_events.c.
 * Independently callable, like tim_ic.h: no vtable, no dispatch.
 */

#ifndef UART_INTERRUPT_H
#define UART_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a link in interrupt mode
 * @note   Re-initializes the handle if it is already open.
 * @param  handle UART handle to populate
 * @param  config Desired configuration
 * @retval UART_OK on success
 */
UART_Status_t UART_Interrupt_Init(UART_Handle_t *handle, const UART_Config_t *config);

/**
 * @brief  Start a send and optionally wait for the TX interrupt to finish it
 * @param  handle  UART handle
 * @param  data    Bytes to send
 * @param  size    Number of bytes
 * @param  timeout Milliseconds to wait, 0 to return immediately
 * @retval UART_OK on success
 */
UART_Status_t UART_Interrupt_Transmit(UART_Handle_t *handle, const uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Start reception and wait for a full packet to drain from the ring
 * @param  handle  UART handle
 * @param  data    Destination buffer
 * @param  size    Number of bytes requested
 * @param  timeout Milliseconds to wait, 0 for a single non-blocking drain
 * @retval UART_OK on success
 */
UART_Status_t UART_Interrupt_Receive(UART_Handle_t *handle, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Re-arm one-shot reception after a completed transfer
 * @details Called by uart_events.c; not part of the app-facing API.
 */
void UART_Interrupt_Rearm(UART_Handle_t *handle);

/**
 * @brief  Rebuild the peripheral after a line error
 * @details Called by uart_events.c; not part of the app-facing API.
 */
void UART_Interrupt_Recover(UART_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_INTERRUPT_H */
