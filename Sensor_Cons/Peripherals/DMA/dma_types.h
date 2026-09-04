/**
 * @file    dma_types.h
 * @brief   Shared types for the DMA driver
 * @details This file holds the basic types and settings for DMA.
 *
 * How it works (in simple words):
 * - DMA moves data from one place to another without using the CPU.
 * - For example, it can move sensor data to memory while the CPU does other work.
 * - You pick the source, the destination, how much to move, and how big each piece is.
 */

#ifndef DMA_TYPES_H
#define DMA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include <stdbool.h>
#include <stdint.h>

/** @brief Transfer item widths accepted by DMA_Config_t::dataSize */
#define DMA_DATA_SIZE_BYTE 0x00U
#define DMA_DATA_SIZE_HALFWORD 0x01U
#define DMA_DATA_SIZE_WORD 0x02U

/** @brief NVIC preemption priority used for every DMA stream interrupt */
#define DMA_IRQ_PRIORITY 5U
#define DMA_IRQ_SUBPRIORITY 0U

/** @brief Widest transfer the 16-bit NDTR counter can express */
#define DMA_MAX_TRANSFER_ITEMS 65535U

/**
 * @brief Settings for one DMA move
 * @details Pick which stream, which channel, which way data goes,
 *          how it repeats, and how big each piece is.
 */
typedef struct {
    DMA_Stream_TypeDef *stream; /**< Which DMA stream to use (e.g. DMA2_Stream0) */
    uint32_t channel;           /**< Which channel (0 to 7) */
    uint32_t direction;     /**< Where data goes (to memory, from memory, or memory to memory) */
    uint32_t mode;          /**< How it runs (once, repeat, or other) */
    uint32_t priority;      /**< How important this move is */
    uint32_t dataSize;      /**< Size of each piece (byte, half-word, word) */
    uint32_t memInc;        /**< Should the memory address move forward? */
    uint32_t periphInc;     /**< Should the device address move forward? */
    uint32_t fifoMode;      /**< Use the small buffer inside DMA? */
    uint32_t fifoThreshold; /**< When to send from that buffer */
} DMA_Config_t;

/**
 * @brief Handle that keeps all DMA info in one place
 * @details Holds the settings, status, and if a move is in progress.
 */
typedef struct {
    DMA_HandleTypeDef hdma;        /**< Low-level DMA handle */
    DMA_Config_t config;           /**< Settings in use */
    bool initialized;              /**< True if ready to use */
    volatile bool busy;            /**< True while data is still moving */
    volatile bool transferStarted; /**< True after the first move has started */
    IRQn_Type irqn;                /**< Which interrupt line this stream uses */
} DMA_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* DMA_TYPES_H */
