/**
 * @file uart_types.h
 * @brief Shared UART vocabulary: status, mode, configuration and handle
 *
 * Single-responsibility module holding only the types every other UART module
 * speaks. It contains no behaviour, so mode modules (blocking, interrupt, DMA)
 * can include it without pulling in the whole driver.
 */

#ifndef UART_TYPES_H
#define UART_TYPES_H

#include "stm32f4xx.h"
#include "uart_ring_buffer.h"
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
    UART_MODE_INTERRUPT, /*!< Byte-by-byte, driven by USART interrupts */
    UART_MODE_DMA        /*!< Buffer transfers offloaded to a DMA stream */
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
 * first. There is no mode vtable: each mode module (blocking, interrupt, DMA)
 * is an independent set of functions, the same way tim_pwm.h/tim_ic.h are.
 * `config.mode` is just a record of which one last initialized this handle,
 * read by uart_events.c to know how to react to a HAL callback.
 */
struct UART_Handle {
    UART_HandleTypeDef *huart;  /*!< HAL UART handle */
    UART_Config_t config;       /*!< Configuration this link was opened with */
    uint8_t *rxBuffer;          /*!< Landing area the HAL receives into */
    uint16_t rxSize;            /*!< Size of rxBuffer */
    RingBuffer_t rxRing;        /*!< Received bytes waiting to be read */
    volatile bool txComplete;   /*!< Raised by the TX callback */
    volatile bool rxComplete;   /*!< Raised by the RX callbacks */
    bool isInitialized;         /*!< Initialization status */
};

#ifdef __cplusplus
}
#endif

#endif /* UART_TYPES_H */
