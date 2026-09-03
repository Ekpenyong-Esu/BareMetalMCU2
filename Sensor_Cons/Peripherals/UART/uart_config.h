/**
 * @file uart_config.h
 * @brief UART tunables for STM32F429I-DISC1
 *
 * Values a board bring-up may want to change. Behaviour lives in the driver
 * modules; this header only holds numbers.
 *
 * Default Configuration:
 * - 115200 baud, 8N1, TX+RX enabled
 * - 5000 ms default timeout for blocking operations
 *
 * DMA Wiring (USART1):
 * - TX: DMA2 Stream 7, Channel 4
 * - RX: DMA2 Stream 5, Channel 4
 * These are consumed by HAL_UART_MspInit() in Core/Src/stm32f4xx_hal_msp.c
 * and must match the CubeMX / hardware configuration.
 */

#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Default UART configuration */
#define UART_DEFAULT_BAUDRATE     115200
#define UART_DEFAULT_WORDLENGTH   UART_WORDLENGTH_8B
#define UART_DEFAULT_STOPBITS     UART_STOPBITS_1
#define UART_DEFAULT_PARITY       UART_PARITY_NONE
#define UART_DEFAULT_MODE         UART_MODE_TX_RX

/* Default timeout value in milliseconds */
#define UART_TIMEOUT         5000  /* Default timeout in milliseconds */

/* DMA wiring for USART1, consumed by HAL_UART_MspInit() in Core */
#define UART_DMA_TX_CHANNEL      DMA_CHANNEL_4
#define UART_DMA_RX_CHANNEL      DMA_CHANNEL_4
#define UART_DMA_TX_STREAM       DMA2_Stream7
#define UART_DMA_RX_STREAM       DMA2_Stream5

#ifdef __cplusplus
}
#endif

#endif /* UART_CONFIG_H */
