/**
 * @file dma.c
 * @brief DMA initialization and lifecycle
 */

#include "dma_core.h"
#include "dma_events.h"
#include "dma_stream.h"
#include "log.h"
#include <string.h>

static bool DMA_ResolveAlignment(uint32_t dataSize, uint32_t *memAlign, uint32_t *periphAlign) {
    switch (dataSize) {
        case DMA_DATA_SIZE_BYTE:
            *memAlign = DMA_MDATAALIGN_BYTE;
            *periphAlign = DMA_PDATAALIGN_BYTE;
            return true;
        case DMA_DATA_SIZE_HALFWORD:
            *memAlign = DMA_MDATAALIGN_HALFWORD;
            *periphAlign = DMA_PDATAALIGN_HALFWORD;
            return true;
        case DMA_DATA_SIZE_WORD:
            *memAlign = DMA_MDATAALIGN_WORD;
            *periphAlign = DMA_PDATAALIGN_WORD;
            return true;
        default:
            return false;
    }
}

/**
 * @brief   Validate a requested configuration
 * @note    The HAL only asserts these values, and assertions are compiled out.
 */
static bool DMA_ValidateConfig(const DMA_Config_t *config) {
    IRQn_Type irqn = NonMaskableInt_IRQn; /* any value: the lookup below sets it */

    if (config == NULL) {
        return false;
    }

    if (!DMA_GetStreamIRQ(config->stream, &irqn)) {
        return false;
    }

    if (config->channel > DMA_CHANNEL_7) {
        return false;
    }

    switch (config->direction) {
        case DMA_PERIPH_TO_MEMORY:
        case DMA_MEMORY_TO_PERIPH:
        case DMA_MEMORY_TO_MEMORY:
            break;
        default:
            return false;
    }

    switch (config->mode) {
        case DMA_NORMAL:
        case DMA_CIRCULAR:
        case DMA_PFCTRL:
            break;
        default:
            return false;
    }

    switch (config->priority) {
        case DMA_PRIORITY_LOW:
        case DMA_PRIORITY_MEDIUM:
        case DMA_PRIORITY_HIGH:
        case DMA_PRIORITY_VERY_HIGH:
            break;
        default:
            return false;
    }

    if ((config->memInc != DMA_MINC_ENABLE) && (config->memInc != DMA_MINC_DISABLE)) {
        return false;
    }

    if ((config->periphInc != DMA_PINC_ENABLE) && (config->periphInc != DMA_PINC_DISABLE)) {
        return false;
    }

    if ((config->fifoMode != DMA_FIFOMODE_ENABLE) && (config->fifoMode != DMA_FIFOMODE_DISABLE)) {
        return false;
    }

    if (config->fifoMode == DMA_FIFOMODE_ENABLE) {
        switch (config->fifoThreshold) {
            case DMA_FIFO_THRESHOLD_1QUARTERFULL:
            case DMA_FIFO_THRESHOLD_HALFFULL:
            case DMA_FIFO_THRESHOLD_3QUARTERSFULL:
            case DMA_FIFO_THRESHOLD_FULL:
                break;
            default:
                return false;
        }
    }

    if (config->direction == DMA_MEMORY_TO_MEMORY) {
        /* RM0090: direct mode and circular mode are both unavailable for
           memory to memory transfers. The HAL does not enforce either. */
        if (config->fifoMode != DMA_FIFOMODE_ENABLE) {
            log_error("DMA: memory to memory requires the FIFO");
            return false;
        }
        if (config->mode != DMA_NORMAL) {
            log_error("DMA: memory to memory requires normal mode");
            return false;
        }
    }

    if (config->mode == DMA_PFCTRL) {
        /* Peripheral flow control is meaningless without a peripheral. */
        if (config->direction == DMA_MEMORY_TO_MEMORY) {
            return false;
        }
    }

    return true;
}

HAL_StatusTypeDef DMA_Init(DMA_Handle_t *handle, const DMA_Config_t *config) {
    uint32_t memAlign = 0;
    uint32_t periphAlign = 0;

    if (handle == NULL) {
        return HAL_ERROR;
    }

    /* Clear before validating so a rejected call cannot leave a reused handle
       reporting itself as initialized. */
    memset(handle, 0, sizeof(*handle));

    if (!DMA_ValidateConfig(config)) {
        log_error("DMA: invalid configuration");
        return HAL_ERROR;
    }

    if (!DMA_ResolveAlignment(config->dataSize, &memAlign, &periphAlign)) {
        return HAL_ERROR;
    }

    handle->config = *config;

    if (DMA_EnableClockAndIRQ(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->hdma.Instance = config->stream;
    handle->hdma.Init.Channel = config->channel;
    handle->hdma.Init.Direction = config->direction;
    handle->hdma.Init.Mode = config->mode;
    handle->hdma.Init.Priority = config->priority;
    handle->hdma.Init.FIFOMode = config->fifoMode;
    handle->hdma.Init.FIFOThreshold = config->fifoThreshold;
    handle->hdma.Init.MemBurst = DMA_MBURST_SINGLE;
    handle->hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
    handle->hdma.Init.MemDataAlignment = memAlign;
    handle->hdma.Init.PeriphDataAlignment = periphAlign;
    handle->hdma.Init.MemInc = config->memInc;
    handle->hdma.Init.PeriphInc = config->periphInc;

    /* Lets the interrupt dispatch find the driver handle from the HAL handle. */
    handle->hdma.Parent = handle;

    if (HAL_DMA_Init(&handle->hdma) != HAL_OK) {
        HAL_NVIC_DisableIRQ(handle->irqn);
        log_error("DMA: HAL_DMA_Init failed");
        return HAL_ERROR;
    }

    if (DMA_Events_Register(handle) != HAL_OK) {
        (void)HAL_DMA_DeInit(&handle->hdma);
        HAL_NVIC_DisableIRQ(handle->irqn);
        log_error("DMA: failed to register transfer callbacks");
        return HAL_ERROR;
    }

    handle->initialized = true;
    log_debug("DMA: initialized");

    return HAL_OK;
}

HAL_StatusTypeDef DMA_DeInit(DMA_Handle_t *handle) {
    if ((handle == NULL) || !handle->initialized) {
        return HAL_ERROR;
    }

    HAL_NVIC_DisableIRQ(handle->irqn);

    if (HAL_DMA_DeInit(&handle->hdma) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->busy = false;
    handle->initialized = false;

    return HAL_OK;
}

bool DMA_IsInitialized(const DMA_Handle_t *handle) {
    return ((handle != NULL) && handle->initialized);
}
