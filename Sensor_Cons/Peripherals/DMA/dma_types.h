/**
 * @file dma_types.h
 * @brief Data types and constants for the DMA driver
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
#define DMA_DATA_SIZE_BYTE       0x00U
#define DMA_DATA_SIZE_HALFWORD   0x01U
#define DMA_DATA_SIZE_WORD       0x02U

/** @brief NVIC preemption priority used for every DMA stream interrupt */
#define DMA_IRQ_PRIORITY         5U
#define DMA_IRQ_SUBPRIORITY      0U

/** @brief Widest transfer the 16-bit NDTR counter can express */
#define DMA_MAX_TRANSFER_ITEMS   65535U

/**
 * @brief DMA stream configuration
 */
typedef struct {
    DMA_Stream_TypeDef *stream;  /**< Stream instance, for example DMA2_Stream0 */
    uint32_t channel;            /**< DMA_CHANNEL_0 to DMA_CHANNEL_7 */
    uint32_t direction;          /**< DMA_PERIPH_TO_MEMORY, DMA_MEMORY_TO_PERIPH or DMA_MEMORY_TO_MEMORY */
    uint32_t mode;               /**< DMA_NORMAL, DMA_CIRCULAR or DMA_PFCTRL */
    uint32_t priority;           /**< DMA_PRIORITY_* */
    uint32_t dataSize;           /**< DMA_DATA_SIZE_BYTE, HALFWORD or WORD */
    uint32_t memInc;             /**< DMA_MINC_ENABLE or DMA_MINC_DISABLE */
    uint32_t periphInc;          /**< DMA_PINC_ENABLE or DMA_PINC_DISABLE */
    uint32_t fifoMode;           /**< DMA_FIFOMODE_ENABLE or DMA_FIFOMODE_DISABLE */
    uint32_t fifoThreshold;      /**< DMA_FIFO_THRESHOLD_* , used only with the FIFO enabled */
} DMA_Config_t;

/**
 * @brief DMA driver handle
 */
typedef struct {
    DMA_HandleTypeDef hdma;      /**< HAL DMA handle */
    DMA_Config_t config;         /**< Configuration in force */
    bool initialized;            /**< Initialization status */
    volatile bool busy;          /**< A transfer has been started and not yet finished */
    volatile bool transferStarted; /**< At least one transfer has been started */
    IRQn_Type irqn;              /**< Interrupt line of the configured stream */
} DMA_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* DMA_TYPES_H */
