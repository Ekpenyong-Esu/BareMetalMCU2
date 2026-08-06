/**
 * @file uart_core.h
 * @brief UART lifecycle and mode-agnostic transfers
 *
 * Single-responsibility module answering the questions every UART user has:
 * how do I open a link, how do I close it, and how do I move bytes without
 * caring whether the link is blocking, interrupt or DMA driven.
 */

#ifndef UART_CORE_H
#define UART_CORE_H

#include "uart_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Open a UART link and bring up its transfer mode
 * @note   Re-initializes the handle if it is already open.
 * @param  handle UART handle to populate
 * @param  config Desired configuration
 * @retval UART_OK on success
 */
UART_Status_t UART_Init(UART_Handle_t *handle, const UART_Config_t *config);

/**
 * @brief  Close a UART link and release its hardware resources
 * @param  handle UART handle
 * @retval UART_OK on success
 */
UART_Status_t UART_DeInit(UART_Handle_t *handle);

/**
 * @brief  Send data using the mode the handle was initialized with
 * @param  handle  UART handle
 * @param  data    Bytes to send
 * @param  size    Number of bytes
 * @param  timeout Milliseconds to wait for completion, 0 to return immediately
 * @retval UART_OK on success
 */
UART_Status_t UART_Transmit(UART_Handle_t *handle, const uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Receive data using the mode the handle was initialized with
 * @param  handle  UART handle
 * @param  data    Destination buffer
 * @param  size    Number of bytes requested
 * @param  timeout Milliseconds to wait for completion, 0 to return immediately
 * @retval UART_OK on success
 */
UART_Status_t UART_Receive(UART_Handle_t *handle, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief  Get the handle of the most recently initialized UART link
 * @details Interrupt vectors and the MSP run without a caller-supplied handle,
 *          so they resolve the owning link through this accessor instead of
 *          reaching for a global defined elsewhere.
 * @retval Active handle, or NULL if no link is open
 */
UART_Handle_t *UART_GetActiveHandle(void);

/**
 * @brief  Spin until a completion flag is raised
 * @param  flag    Flag an interrupt callback will set
 * @param  timeout Milliseconds to wait; 0 returns immediately
 * @retval UART_OK once raised, UART_TIMEOUT_ERROR if the wait expired
 */
UART_Status_t UART_WaitForFlag(volatile bool *flag, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* UART_CORE_H */
