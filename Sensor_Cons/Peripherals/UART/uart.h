/**
 * @file uart.h
 * @brief Shared UART utilities: teardown and active-link lookup
 *
 * Not a dispatcher: each transfer mode (uart_blocking.h/uart_interrupt.h/
 * uart_dma.h) is an independent, directly-callable API, the same way
 * tim_pwm.h/tim_ic.h are. This header only holds the handful of things that
 * are genuinely mode-agnostic, the UART equivalent of tim_clock.h.
 */

#ifndef UART_H
#define UART_H

#include "uart_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Close a UART link and release its hardware resources
 * @param  handle UART handle
 * @retval UART_OK on success
 */
UART_Status_t UART_DeInit(UART_Handle_t *handle);

/**
 * @brief  Publish the link an Init() just brought up
 * @details Interrupt vectors and the MSP run without a caller-supplied handle,
 *          so each mode's Init() calls this before HAL_UART_Init() so that
 *          HAL_UART_MspInit() and the ISR callbacks can find the link.
 * @param  handle Handle to publish, or NULL to clear it
 */
void UART_SetActiveHandle(UART_Handle_t *handle);

/**
 * @brief  Get the handle of the most recently initialized UART link
 * @retval Active handle, or NULL if no link is open
 */
UART_Handle_t *UART_GetActiveHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
