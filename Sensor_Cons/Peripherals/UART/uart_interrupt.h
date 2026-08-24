/**
 * @file uart_interrupt.h
 * @brief Interrupt-driven transfer mode
 *
 * Nothing here waits. Init() arms reception and it stays armed: the callbacks
 * in uart_events.c park each chunk in the handle's ring buffer and re-arm
 * immediately, so bytes keep arriving while the main loop is busy elsewhere.
 * Read() takes whatever has piled up and returns at once; Write() hands the
 * peripheral a buffer and returns while it is still going out.
 *
 * That is the whole difference from uart_blocking.h. If you find yourself
 * spinning on these calls, you wanted blocking mode.
 *
 * Independently callable, like tim_ic.h: no vtable, no dispatch.
 */

#ifndef UART_INTERRUPT_H
#define UART_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

/**
 * @brief  Open a link in interrupt mode and start listening
 * @note   Re-initializes the handle if it is already open.
 * @param  rxBuffer     Landing area the ISR receives into; must outlive the
 *                      handle and belongs to the driver until DeInit
 * @param  rxBufferSize Size of @p rxBuffer, at most RING_BUFFER_SIZE
 * @retval UART_OK on success
 */
UART_Status_t UART_Interrupt_Init(UART_Handle_t *handle, const UART_Config_t *config,
                                  uint8_t *rxBuffer, uint16_t rxBufferSize);

/**
 * @brief  Take whatever has been received so far, up to @p size bytes
 * @note   Returns immediately. A quiet line is not an error: @p received is
 *         set to 0 and the status is still UART_OK. Bytes past @p received
 *         are untouched, so never treat @p data as a string.
 * @param  received Out: bytes actually written, set even on failure
 * @retval UART_OK unless the arguments were wrong
 */
UART_Status_t UART_Interrupt_Read(UART_Handle_t *handle, uint8_t *data, uint16_t size,
                                  uint16_t *received);

/**
 * @brief  Start sending and return; the ISR does the work
 * @note   @p data belongs to the driver until UART_Interrupt_IsTxDone() goes
 *         true. Reusing or freeing it before then corrupts the transmission.
 * @retval UART_BUSY if the previous send is still in flight
 */
UART_Status_t UART_Interrupt_Write(UART_Handle_t *handle, const uint8_t *data, uint16_t size);

/**
 * @brief  Whether the last Write() has finished leaving the peripheral
 * @note   True when idle, so it is safe to call before the first Write().
 */
bool UART_Interrupt_IsTxDone(const UART_Handle_t *handle);

/**
 * @brief  Re-arm reception after a completed transfer
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
