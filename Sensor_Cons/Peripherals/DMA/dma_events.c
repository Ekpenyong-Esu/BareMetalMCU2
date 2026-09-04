/**
 * @file dma_events.c
 * @brief DMA interrupt dispatch and completion callbacks
 */

#include "dma_events.h"

/*
 * The user callbacks are registered indirectly so the driver can clear the
 * busy flag first. Registering them straight with the HAL, as the previous
 * implementation did, left the driver with no record of when a transfer ended.
 */
static void DMA_OnTransferComplete(DMA_HandleTypeDef *hdma) {
    DMA_Handle_t *handle = (DMA_Handle_t *)hdma->Parent;

    if (handle != NULL) {
        handle->busy = false;
    }

    DMA_TransferCompleteCallback(hdma);
}

static void DMA_OnTransferError(DMA_HandleTypeDef *hdma) {
    DMA_Handle_t *handle = (DMA_Handle_t *)hdma->Parent;

    if (handle != NULL) {
        handle->busy = false;
    }

    DMA_TransferErrorCallback(hdma);
}

static void DMA_OnAbort(DMA_HandleTypeDef *hdma) {
    DMA_Handle_t *handle = (DMA_Handle_t *)hdma->Parent;

    if (handle != NULL) {
        handle->busy = false;
    }
}

HAL_StatusTypeDef DMA_Events_Register(DMA_Handle_t *handle) {
    if (handle == NULL) {
        return HAL_ERROR;
    }

    if (HAL_DMA_RegisterCallback(&handle->hdma, HAL_DMA_XFER_CPLT_CB_ID, DMA_OnTransferComplete) !=
        HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_DMA_RegisterCallback(&handle->hdma, HAL_DMA_XFER_ERROR_CB_ID, DMA_OnTransferError) !=
        HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_DMA_RegisterCallback(&handle->hdma, HAL_DMA_XFER_ABORT_CB_ID, DMA_OnAbort);
}

void DMA_IRQHandler(DMA_Handle_t *handle) {
    if ((handle != NULL) && handle->initialized) {
        HAL_DMA_IRQHandler(&handle->hdma);
    }
}

__weak void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma) {
    UNUSED(hdma);
}

__weak void DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma) {
    UNUSED(hdma);
}
