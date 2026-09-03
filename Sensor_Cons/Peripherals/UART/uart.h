/**
 * @file uart.h
 * @brief Shared UART utilities: teardown, active-link lookup, flag waits
 *
 * Not a dispatcher: each transfer mode (uart_blocking.h/uart_interrupt.h/
 * uart_dma.h) is an independent, directly-callable API, the same way
 * tim_pwm.h/tim_ic.h are. This header only holds the handful of things that
 * are genuinely mode-agnostic, the UART equivalent of tim_clock.h.
 *
 * Architecture:
 * - Each UART link is a UART_Handle_t containing HAL handle, config, and state
 * - Modes are independent: blocking, interrupt, DMA - no vtable dispatch
 * - Active handle registry allows MSP/ISR to find the right link
 * - Ring buffer (uart_ring_buffer) decouples ISR reception from application reads
 */

#ifndef UART_H
#define UART_H

#include "uart_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Close a UART link and release its hardware resources
 *
 * Disables all UART interrupts, calls HAL_UART_DeInit() to release MSP
 * resources (GPIO, clocks, DMA), and clears the handle state. If this
 * handle was the active one, the active handle registry is cleared.
 *
 * @param  handle UART handle to deinitialize
 * @retval UART_OK on success, UART_ERROR if handle/huart is NULL or HAL fails
 */
UART_Status_t UART_DeInit(UART_Handle_t *handle);

/**
 * @brief  Publish the link an Init() just brought up
 *
 * Interrupt vectors and HAL_UART_MspInit() run without a caller-supplied
 * handle, so each mode's Init() calls this before HAL_UART_Init() so that
 * the MSP and ISR callbacks can find the link via UART_GetActiveHandle().
 *
 * @param  handle Handle to publish, or NULL to clear the active handle
 */
void UART_SetActiveHandle(UART_Handle_t *handle);

/**
 * @brief  Get the handle of the most recently initialized UART link
 *
 * @retval Active handle, or NULL if no link is open
 */
UART_Handle_t *UART_GetActiveHandle(void);

/**
 * @brief  Spin until a completion flag is raised
 *
 * Busy-waits on a volatile boolean flag (typically txComplete or rxComplete
 * set by interrupt callbacks). Uses HAL_GetTick() for timeout.
 *
 * @param  flag    Pointer to volatile flag that an interrupt callback will set
 * @param  timeout Milliseconds to wait; 0 returns UART_TIMEOUT_ERROR unless
 *                 the flag is already raised
 * @retval UART_OK once raised, UART_TIMEOUT_ERROR if the wait expired
 */
UART_Status_t UART_WaitForFlag(volatile bool *flag, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
