/**
 * @file    log_sink_uart.c
 * @brief   UART log backend
 */

#include "log_sink_uart.h"

static UART_HandleTypeDef *LogSinkUart_Link = NULL;

static void LogSinkUart_Write(const char *text, size_t length) {
    if (LogSinkUart_Link == NULL || length == 0u) {
        return;
    }
    HAL_UART_Transmit(LogSinkUart_Link, (const uint8_t *)text, (uint16_t)length,
                      LOG_UART_TIMEOUT_MS);
}

static const log_sink_t LogSinkUart = {
    .name = "uart",
    .write = LogSinkUart_Write,
};

const log_sink_t *log_sink_uart(UART_HandleTypeDef *huart) {
    LogSinkUart_Link = huart;
    return &LogSinkUart;
}
