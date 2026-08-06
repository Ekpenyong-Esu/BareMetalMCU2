/**
 * @file uart_interrupt.h
 * @brief Interrupt-driven transfer mode
 *
 * Transfers are started here and completed by the callbacks in uart_events.c.
 */

#ifndef UART_INTERRUPT_H
#define UART_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

extern const UART_ModeOps_t UART_InterruptOps;

#ifdef __cplusplus
}
#endif

#endif /* UART_INTERRUPT_H */
