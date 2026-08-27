/**
 * @file uart_types.h
 * @brief Shared UART vocabulary: status, mode, configuration and handle
 *
 * Single-responsibility module holding only the types every other UART module
 * speaks. It contains no behaviour, so the mode module (uart_blocking.h) can
 * include it without pulling in the whole driver.
 */

#ifndef UART_TYPES_H
#define UART_TYPES_H

#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief How bytes are moved between the peripheral and memory
 */
typedef enum {
    UART_MODE_BLOCKING,  /*!< CPU waits in the transfer call */
} UART_Mode_t;

/**
 * @brief Result of a UART operation
 */
typedef enum {
    UART_OK = 0,       /*!< Operation successful */
    UART_ERROR,        /*!< Generic error */
    UART_BUSY,         /*!< UART is busy */
    UART_TIMEOUT_ERROR /*!< Operation timed out */
} UART_Status_t;

/**
 * @brief Everything the caller chooses about a UART link
 */
typedef struct {
    USART_TypeDef *instance; /*!< UART instance (USART1, USART2, ...) */
    uint32_t baudRate;       /*!< Baud rate */
    uint8_t wordLength;      /*!< Word length (8 or 9 bits) */
    uint8_t stopBits;        /*!< Number of stop bits */
    uint8_t parity;          /*!< Parity mode */
    UART_Mode_t mode;        /*!< Transfer mode */
} UART_Config_t;

typedef struct UART_Handle UART_Handle_t;

/**
 * @brief Runtime state of one UART link
 *
 * All per-link state lives here, so opening a second link cannot disturb the
 * first. `config.mode` records which mode module last initialized this handle.
 */
struct UART_Handle {
    UART_HandleTypeDef *huart;  /*!< HAL UART handle */
    UART_Config_t config;       /*!< Configuration this link was opened with */
    bool isInitialized;         /*!< Initialization status */
};

#ifdef __cplusplus
}
#endif

#endif /* UART_TYPES_H */
