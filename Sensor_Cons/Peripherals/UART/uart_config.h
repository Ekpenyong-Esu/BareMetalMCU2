/**
 * @file uart_config.h
 * @brief UART tunables
 *
 * Values a board bring-up may want to change. Behaviour lives in the driver
 * modules; this header only holds numbers.
 *
 * Default Configuration:
 * - 115200 baud, 8N1, TX+RX enabled
 * - 5000 ms default timeout for blocking operations
 *
 * Wiring (instance, pins, DMA streams) is not a tunable: the application
 * passes it in UART_Config_t when it opens the link.
 */

#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Default UART configuration */
#define UART_DEFAULT_BAUDRATE 115200
#define UART_DEFAULT_WORDLENGTH UART_WORDLENGTH_8B
#define UART_DEFAULT_STOPBITS UART_STOPBITS_1
#define UART_DEFAULT_PARITY UART_PARITY_NONE
#define UART_DEFAULT_MODE UART_MODE_TX_RX

/* Default timeout value in milliseconds */
#define UART_TIMEOUT 5000 /* Default timeout in milliseconds */

#ifdef __cplusplus
}
#endif

#endif /* UART_CONFIG_H */
