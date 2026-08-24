/**
 * @file    log_sink_uart.h
 * @brief   Log backend writing through a UART link
 * @details Takes a HAL handle rather than the repo UART driver so logging stays
 *          usable from any UART instance and adds no driver dependency.
 */

#ifndef LOG_SINK_UART_H
#define LOG_SINK_UART_H

#include "stm32f4xx_hal.h"
#include "log_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Milliseconds a log line may spend waiting for the UART before it is dropped. */
#ifndef LOG_UART_TIMEOUT_MS
#define LOG_UART_TIMEOUT_MS 100U
#endif

/**
 * @brief  Point the UART backend at a link and get it back ready to install
 * @param  huart Initialized HAL UART handle, or NULL to disable the backend
 * @retval Sink to hand to log_set_sink()
 */
const log_sink_t *log_sink_uart(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* LOG_SINK_UART_H */
