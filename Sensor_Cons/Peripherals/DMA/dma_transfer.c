/**
 * @file dma_transfer.c
 * @brief DMA transfer control and status
 */

#include "dma_transfer.h"
#include "dma_core.h"
#include "log.h"

static uint32_t DMA_GetItemSize(const DMA_Handle_t *handle) {
    switch (handle->config.dataSize) {
        case DMA_DATA_SIZE_HALFWORD:
            return 2U;
        case DMA_DATA_SIZE_WORD:
            return 4U;
        default:
            return 1U;
    }
}

HAL_StatusTypeDef DMA_StartTransfer(DMA_Handle_t *handle, uint32_t srcAddr, uint32_t dstAddr,
                                    uint32_t dataLength) {
    uint32_t itemSize = 0;
    HAL_StatusTypeDef status = HAL_OK;

    if (!DMA_IsInitialized(handle)) {
        return HAL_ERROR;
    }

    if ((srcAddr == 0U) || (dstAddr == 0U) || (dataLength == 0U)) {
        return HAL_ERROR;
    }

    /* NDTR is 16 bits wide; a larger count silently wraps. */
    if (dataLength > DMA_MAX_TRANSFER_ITEMS) {
        log_error("DMA: transfer of %lu items exceeds the NDTR range", (unsigned long)dataLength);
        return HAL_ERROR;
    }

    itemSize = DMA_GetItemSize(handle);
    if (((srcAddr % itemSize) != 0U) || ((dstAddr % itemSize) != 0U)) {
        log_error("DMA: addresses must be aligned to the %lu byte item size",
                  (unsigned long)itemSize);
        return HAL_ERROR;
    }

    if (handle->busy) {
        return HAL_BUSY;
    }

    handle->busy = true;
    handle->transferStarted = true;

    status = HAL_DMA_Start_IT(&handle->hdma, srcAddr, dstAddr, dataLength);
    if (status != HAL_OK) {
        handle->busy = false;
    }

    return status;
}

HAL_StatusTypeDef DMA_StartPeriphToMem(DMA_Handle_t *handle, uint32_t periphAddr, uint32_t memAddr,
                                       uint32_t dataLength) {
    if (!DMA_IsInitialized(handle) || (handle->config.direction != DMA_PERIPH_TO_MEMORY)) {
        return HAL_ERROR;
    }

    return DMA_StartTransfer(handle, periphAddr, memAddr, dataLength);
}

HAL_StatusTypeDef DMA_StartMemToPeriph(DMA_Handle_t *handle, uint32_t memAddr, uint32_t periphAddr,
                                       uint32_t dataLength) {
    if (!DMA_IsInitialized(handle) || (handle->config.direction != DMA_MEMORY_TO_PERIPH)) {
        return HAL_ERROR;
    }

    return DMA_StartTransfer(handle, memAddr, periphAddr, dataLength);
}

HAL_StatusTypeDef DMA_StopTransfer(DMA_Handle_t *handle) {
    HAL_StatusTypeDef status = HAL_OK;

    if (!DMA_IsInitialized(handle)) {
        return HAL_ERROR;
    }

    /* HAL_DMA_Abort is blocking and raises no callback, so clear the flag here. */
    status = HAL_DMA_Abort(&handle->hdma);
    handle->busy = false;

    return status;
}

bool DMA_IsTransferComplete(const DMA_Handle_t *handle) {
    if (!DMA_IsInitialized(handle)) {
        return false;
    }

    return (handle->transferStarted && !handle->busy);
}

bool DMA_IsBusy(const DMA_Handle_t *handle) {
    if (!DMA_IsInitialized(handle)) {
        return false;
    }

    return handle->busy;
}

uint32_t DMA_GetError(const DMA_Handle_t *handle) {
    if (!DMA_IsInitialized(handle)) {
        return HAL_DMA_ERROR_NONE;
    }

    /* Read the field directly; HAL_DMA_GetError takes a non-const handle. */
    return handle->hdma.ErrorCode;
}
