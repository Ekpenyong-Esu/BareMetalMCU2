/**
 * @file uart_blocking.h
 * @brief Blocking transfer mode
 *
 * The CPU waits inside each transfer. Nothing runs in interrupt context, so
 * this mode implements only the three mandatory operations.
 */

#ifndef UART_BLOCKING_H
#define UART_BLOCKING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

extern const UART_ModeOps_t UART_BlockingOps;

#ifdef __cplusplus
}
#endif

#endif /* UART_BLOCKING_H */
