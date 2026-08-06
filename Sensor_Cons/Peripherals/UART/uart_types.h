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

#include "stm32f4xx_hal.h"
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
 * @brief What a transfer mode has to be able to do
 *
 * Resolved once by UART_Init(), which is why no other function needs to switch
 * on UART_Mode_t. Optional entries are NULL when the mode has nothing to do:
 * blocking mode never runs in interrupt context, so it fills in neither
 * rearmReceive, completedReceiveSize nor recoverFromError.
 */
typedef struct {
    const char *name; /*!< Mode name, for logs */

    UART_Status_t (*init)(UART_Handle_t *handle);
    UART_Status_t (*transmit)(UART_Handle_t *handle, const uint8_t *data, uint16_t size, uint32_t timeout);
    UART_Status_t (*receive)(UART_Handle_t *handle, uint8_t *data, uint16_t size, uint32_t timeout);

    /** Re-arm one-shot reception. NULL marks a mode that never receives asynchronously. */
    void (*rearmReceive)(UART_Handle_t *handle);

    /** Bytes the HAL has just delivered into rxBuffer, asked at RxCplt time. */
    uint16_t (*completedReceiveSize)(const UART_Handle_t *handle, const UART_HandleTypeDef *huart);

    /** Extra work needed to make the peripheral usable again after a line error. */
    void (*recoverFromError)(UART_Handle_t *handle);
} UART_ModeOps_t;

/**
 * @brief Runtime state of one UART link
 *
 * All per-link state lives here, so opening a second link cannot disturb the
 * first.
 */
struct UART_Handle {
    UART_HandleTypeDef *huart;  /*!< HAL UART handle */
    UART_Config_t config;       /*!< Configuration this link was opened with */
    const UART_ModeOps_t *ops;  /*!< Transfer mode, resolved by UART_Init() */
    uint8_t *rxBuffer;          /*!< Landing area the HAL receives into */
    uint8_t *txBuffer;          /*!< Transmit buffer */
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
