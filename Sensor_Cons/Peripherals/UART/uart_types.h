/**
 * @file    uart_types.h
 * @brief   Shared types for the UART driver
 * @details UART sends text and bytes over two wires. One wire sends,
 *          one wire receives. It is like a serial chat between boards.
 *          This file holds the common types and settings used by the
 *          UART driver.
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
    UART_MODE_BLOCKING,  /*!< CPU waits in the transfer call (HAL_UART_Transmit/Receive) */
    UART_MODE_INTERRUPT, /*!< Byte-by-byte, driven by USART interrupts
                            (HAL_UART_Transmit_IT/Receive_IT) */
    UART_MODE_DMA        /*!< Buffer transfers offloaded to a DMA stream
                            (HAL_UART_Transmit_DMA/Receive_DMA) */
} UART_Mode_t;

/**
 * @brief Result of a UART operation
 */
typedef enum {
    UART_OK = 0,       /*!< Operation successful */
    UART_ERROR,        /*!< Generic error (HAL error, invalid args, etc.) */
    UART_BUSY,         /*!< UART is busy (another transfer in progress) */
    UART_TIMEOUT_ERROR /*!< Operation timed out */
} UART_Status_t;

/**
 * @brief Everything the caller chooses about a UART link
 *
 * The driver owns no board detail: the application names the instance, the
 * pins and, for DMA mode, the streams. A NULL port means that direction is
 * not wired; at least one direction must be. `alternate` 0 lets the driver
 * derive the AF from the instance (USART1/2/3 = AF7, UART4/5/USART6 = AF8).
 * The DMA fields are only read when `mode` is UART_MODE_DMA.
 */
typedef struct {
    USART_TypeDef *instance; /*!< UART instance (USART1, USART2, USART3, UART4, UART5, USART6) */
    GPIO_TypeDef *txPort;    /*!< TX pin port, or NULL when TX is not wired */
    uint16_t txPin;          /*!< TX pin (GPIO_PIN_x) */
    GPIO_TypeDef *rxPort;    /*!< RX pin port, or NULL when RX is not wired */
    uint16_t rxPin;          /*!< RX pin (GPIO_PIN_x) */
    uint8_t alternate;       /*!< GPIO alternate function, 0 = derive from instance */
    DMA_Stream_TypeDef *dmaTxStream; /*!< DMA stream for TX (DMA mode only) */
    uint32_t dmaTxChannel;           /*!< DMA channel for TX (DMA_CHANNEL_x, DMA mode only) */
    DMA_Stream_TypeDef *dmaRxStream; /*!< DMA stream for RX (DMA mode only) */
    uint32_t dmaRxChannel;           /*!< DMA channel for RX (DMA_CHANNEL_x, DMA mode only) */
    uint32_t baudRate;               /*!< Baud rate (e.g., 115200, 921600) */
    uint8_t wordLength;              /*!< Word length: UART_WORDLENGTH_8B or UART_WORDLENGTH_9B */
    uint8_t stopBits;                /*!< Stop bits: UART_STOPBITS_1, UART_STOPBITS_2, etc. */
    uint8_t parity;   /*!< Parity: UART_PARITY_NONE, UART_PARITY_EVEN, UART_PARITY_ODD */
    UART_Mode_t mode; /*!< Transfer mode (recorded for callback routing) */
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
    UART_HandleTypeDef *huart; /*!< HAL UART handle (allocated by caller) */
    UART_Config_t config;      /*!< Configuration this link was opened with */
    DMA_HandleTypeDef hdmaTx;  /*!< HAL DMA handle for TX, linked by the MSP in DMA mode */
    DMA_HandleTypeDef hdmaRx;  /*!< HAL DMA handle for RX, linked by the MSP in DMA mode */
    uint8_t *rxBuffer;         /*!< Landing area the HAL receives into (allocated by caller) */
    uint16_t rxSize;           /*!< Size of rxBuffer in bytes */
    RingBuffer_t rxRing;       /*!< Received bytes waiting to be read (interrupt/DMA modes) */
    volatile bool txComplete;  /*!< Raised by the TX complete callback */
    volatile bool rxComplete;  /*!< Raised by the RX complete/IDLE callbacks */
    bool isInitialized;        /*!< Initialization status */
};

#ifdef __cplusplus
}
#endif

#endif /* UART_TYPES_H */
