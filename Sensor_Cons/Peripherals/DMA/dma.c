/**
 * @file dma.c
 * @brief DMA Driver Implementation
 * @author Generated for STM32F429
 * @date 2025
 *
 * This file implements the DMA driver functions for STM32F4 series.
 */

#include "dma.h"
#include "log.h"

/** @brief NVIC preemption priority used for every DMA stream interrupt */
#define DMA_IRQ_PRIORITY    5U

/**
 * @brief Get DMA stream IRQ number from stream instance
 */
IRQn_Type DMA_GetStreamIRQ(DMA_Stream_TypeDef *stream) {
    /* DMA1 Streams */
    if (stream == DMA1_Stream0) { return DMA1_Stream0_IRQn; }
    if (stream == DMA1_Stream1) { return DMA1_Stream1_IRQn; }
    if (stream == DMA1_Stream2) { return DMA1_Stream2_IRQn; }
    if (stream == DMA1_Stream3) { return DMA1_Stream3_IRQn; }
    if (stream == DMA1_Stream4) { return DMA1_Stream4_IRQn; }
    if (stream == DMA1_Stream5) { return DMA1_Stream5_IRQn; }
    if (stream == DMA1_Stream6) { return DMA1_Stream6_IRQn; }
    if (stream == DMA1_Stream7) { return DMA1_Stream7_IRQn; }

    /* DMA2 Streams */
    if (stream == DMA2_Stream0) { return DMA2_Stream0_IRQn; }
    if (stream == DMA2_Stream1) { return DMA2_Stream1_IRQn; }
    if (stream == DMA2_Stream2) { return DMA2_Stream2_IRQn; }
    if (stream == DMA2_Stream3) { return DMA2_Stream3_IRQn; }
    if (stream == DMA2_Stream4) { return DMA2_Stream4_IRQn; }
    if (stream == DMA2_Stream5) { return DMA2_Stream5_IRQn; }
    if (stream == DMA2_Stream6) { return DMA2_Stream6_IRQn; }
    if (stream == DMA2_Stream7) { return DMA2_Stream7_IRQn; }

    /* Invalid stream */
    return (IRQn_Type)-1;
}

/**
 * @brief Enable DMA clock and configure NVIC
 */
HAL_StatusTypeDef DMA_EnableClockAndIRQ(DMA_Handle_t *handle) {
    if (handle == NULL || handle->config.stream == NULL) {
        return HAL_ERROR;
    }

    /* Enable DMA clock based on DMA instance */
    if (((uint32_t)handle->config.stream >= (uint32_t)DMA1_Stream0) &&
        ((uint32_t)handle->config.stream <= (uint32_t)DMA1_Stream7)) {
        /* DMA1 */
        __HAL_RCC_DMA1_CLK_ENABLE();
    } else if (((uint32_t)handle->config.stream >= (uint32_t)DMA2_Stream0) &&
               ((uint32_t)handle->config.stream <= (uint32_t)DMA2_Stream7)) {
        /* DMA2 */
        __HAL_RCC_DMA2_CLK_ENABLE();
    } else {
        return HAL_ERROR;
    }

    /* Get IRQ number */
    handle->irqn = DMA_GetStreamIRQ(handle->config.stream);
    if (handle->irqn == (IRQn_Type)-1) {
        return HAL_ERROR;
    }

    /* Configure NVIC */
    HAL_NVIC_SetPriority(handle->irqn, DMA_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(handle->irqn);

    return HAL_OK;
}

/**
 * @brief Initialize DMA with given configuration
 */
HAL_StatusTypeDef DMA_Init(DMA_Handle_t *handle, DMA_Config_t *config) {
    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }

    log_debug("DMA: Initializing DMA");

    /* Validate configuration parameters */
    if (config->stream == NULL) {
        return HAL_ERROR;
    }

    /* Copy configuration into handle first */
    handle->config = *config;

    /* Enable DMA clock and configure IRQ (requires handle->config.stream) */
    if (DMA_EnableClockAndIRQ(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Initialize HAL DMA handle */
    handle->hdma.Instance = config->stream;
    handle->hdma.Init.Channel = config->channel;
    handle->hdma.Init.Direction = config->direction;
    handle->hdma.Init.Mode = config->mode;
    handle->hdma.Init.Priority = config->priority;
    handle->hdma.Init.FIFOMode = config->fifoMode;
    handle->hdma.Init.FIFOThreshold = config->fifoThreshold;
    handle->hdma.Init.MemBurst = DMA_MBURST_SINGLE;
    handle->hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;

    /* Set data sizes */
    switch (config->dataSize) {
        case DMA_DATA_SIZE_BYTE:
            handle->hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
            handle->hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            break;
        case DMA_DATA_SIZE_HALFWORD:
            handle->hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
            handle->hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
            break;
        case DMA_DATA_SIZE_WORD:
            handle->hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
            handle->hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
            break;
        default:
            return HAL_ERROR;
    }

    /* Set increment modes */
    handle->hdma.Init.MemInc = config->memInc;
    handle->hdma.Init.PeriphInc = config->periphInc;

    /* Store user data for callbacks */
    handle->hdma.Parent = (void *)handle;

    /* Initialize DMA */
    if (HAL_DMA_Init(&handle->hdma) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Register callbacks */
    if (HAL_DMA_RegisterCallback(&handle->hdma, HAL_DMA_XFER_CPLT_CB_ID, DMA_TransferCompleteCallback) != HAL_OK ||
        HAL_DMA_RegisterCallback(&handle->hdma, HAL_DMA_XFER_ERROR_CB_ID, DMA_TransferErrorCallback) != HAL_OK) {
        log_error("DMA: failed to register transfer callbacks");
        return HAL_ERROR;
    }

    handle->initialized = true;

    log_debug("DMA: DMA initialized successfully");

    return HAL_OK;
}

/**
 * @brief Deinitialize DMA
 */
HAL_StatusTypeDef DMA_DeInit(DMA_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Disable NVIC */
    HAL_NVIC_DisableIRQ(handle->irqn);

    if (HAL_DMA_DeInit(&handle->hdma) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->initialized = false;
    return HAL_OK;
}

/**
 * @brief DMA IRQ Handler - to be called from DMA stream IRQ handler
 */
void DMA_IRQHandler(DMA_Handle_t *handle) {
    if (handle != NULL && handle->initialized) {
        HAL_DMA_IRQHandler(&handle->hdma);
    }
}

/**
 * @brief Start DMA transfer (Memory to Memory)
 */
HAL_StatusTypeDef DMA_StartTransfer(DMA_Handle_t *handle, uint32_t srcAddr, uint32_t dstAddr, uint32_t dataLength) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    if (srcAddr == 0 || dstAddr == 0 || dataLength == 0) {
        return HAL_ERROR;
    }

    return HAL_DMA_Start_IT(&handle->hdma, srcAddr, dstAddr, dataLength);
}

/**
 * @brief Start DMA transfer (Peripheral to Memory)
 */
HAL_StatusTypeDef DMA_StartPeriphToMem(DMA_Handle_t *handle, uint32_t periphAddr, uint32_t memAddr, uint32_t dataLength) {
    /* The transfer direction is fixed by DMA_Init(); this name only documents
       the intended argument order at the call site. */
    return DMA_StartTransfer(handle, periphAddr, memAddr, dataLength);
}

/**
 * @brief Start DMA transfer (Memory to Peripheral)
 */
HAL_StatusTypeDef DMA_StartMemToPeriph(DMA_Handle_t *handle, uint32_t memAddr, uint32_t periphAddr, uint32_t dataLength) {
    /* The transfer direction is fixed by DMA_Init(); this name only documents
       the intended argument order at the call site. */
    return DMA_StartTransfer(handle, memAddr, periphAddr, dataLength);
}

/**
 * @brief Stop DMA transfer
 */
HAL_StatusTypeDef DMA_StopTransfer(DMA_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    return HAL_DMA_Abort(&handle->hdma);
}

/**
 * @brief Check if DMA transfer is complete
 */
bool DMA_IsTransferComplete(DMA_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    return (HAL_DMA_GetState(&handle->hdma) == HAL_DMA_STATE_READY);
}

/**
 * @brief Get DMA error status
 */
uint32_t DMA_GetError(DMA_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return 0;
    }

    return HAL_DMA_GetError(&handle->hdma);
}

/**
 * @brief DMA Transfer Complete Callback (weak implementation)
 */
__weak void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma) {
    /* User should implement this callback */
    UNUSED(hdma);
}

/**
 * @brief DMA Transfer Error Callback (weak implementation)
 */
__weak void DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma) {
    /* User should implement this callback */
    UNUSED(hdma);
}
