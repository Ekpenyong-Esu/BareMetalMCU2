/**
 * @file uart.h
 * @brief Shared UART utilities: teardown, instance registry, vector dispatch, flag waits
 *
 * Not a dispatcher: each transfer mode (uart_blocking.h/uart_interrupt.h/
 * uart_dma.h) is an independent, directly-callable API, the same way
 * tim_pwm.h/tim_ic.h are. This header only holds the handful of things that
 * are genuinely mode-agnostic, the UART equivalent of tim_clock.h.
 *
 * Architecture:
 * - Each UART link is a UART_Handle_t containing HAL handle, config, and state
 * - Modes are independent: blocking, interrupt, DMA - no vtable dispatch
 * - A registry keyed on the USART instance lets the MSP, the HAL callbacks
 *   and the interrupt vectors find the owning link, so several UARTs can be
 *   open at once without mis-routing
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
 * resources (GPIO, clocks, DMA), removes the link from the registry and
 * clears the handle state.
 *
 * @param  handle UART handle to deinitialize
 * @retval UART_OK on success, UART_ERROR if handle/huart is NULL or HAL fails
 */
UART_Status_t UART_DeInit(UART_Handle_t *handle);

/**
 * @brief  Register a link under its instance before HAL_UART_Init()
 *
 * HAL_UART_MspInit() and the interrupt vectors run without a caller-supplied
 * handle, so each mode's Init() registers the link first. Registration also
 * checks that the hardware binding in `handle->config` is complete: a known
 * instance, at least one wired direction, and both DMA streams in DMA mode.
 *
 * @param  handle Handle whose config has already been filled in
 * @retval UART_OK when registered, UART_ERROR when the binding is invalid or
 *         the instance is already owned by another open link
 */
UART_Status_t UART_Register(UART_Handle_t *handle);

/**
 * @brief  Remove a link from the registry
 *
 * @param  handle Handle to remove; a handle that is not registered is ignored
 */
void UART_Unregister(UART_Handle_t *handle);

/**
 * @brief  Resolve the link that owns a USART instance
 *
 * @param  instance USART1..USART6 / UART4 / UART5
 * @retval Owning handle, or NULL when that instance is not open
 */
UART_Handle_t *UART_FromInstance(const USART_TypeDef *instance);

/**
 * @brief  Service the USART interrupt vector for one instance
 *
 * Called from the USARTx_IRQHandler() in Core; routes to HAL_UART_IRQHandler()
 * on the registered link, or does nothing when the instance is not open.
 *
 * @param  instance USART instance whose vector fired
 */
void UART_IRQHandler(USART_TypeDef *instance);

/**
 * @brief  Service a DMA stream interrupt vector on behalf of a UART link
 *
 * Called from the DMAx_StreamN_IRQHandler() in Core; finds the open link whose
 * TX or RX stream is @p stream and routes to HAL_DMA_IRQHandler(), or does
 * nothing when no UART owns that stream.
 *
 * @param  stream DMA stream whose vector fired
 */
void UART_DmaStreamIRQHandler(const DMA_Stream_TypeDef *stream);

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
UART_Status_t UART_WaitForFlag(const volatile bool *flag, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
