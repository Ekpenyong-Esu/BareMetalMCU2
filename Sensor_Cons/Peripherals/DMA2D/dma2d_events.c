/**
 * @file dma2d_events.c
 * @brief DMA2D interrupt context handlers and callback dispatch
 *
 * @details
 * All code that runs at interrupt priority lives here. The HAL callbacks below
 * are weak-symbol overrides and are never called directly by the driver.
 */

#include "dma2d_events.h"
#include "dma2d_core.h"
#include "log.h"

/* Private variables ---------------------------------------------------------*/

static DMA2D_TransferCompleteCallback s_completeCallback = NULL;
static DMA2D_TransferErrorCallback    s_errorCallback = NULL;
static DMA2D_TransferProgressCallback s_progressCallback = NULL;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Report whether a HAL handle belongs to this driver
 * @param handle Handle supplied by the HAL callback
 * @return true when the handle is the driver's own
 */
static bool DMA2D_IsOwnHandle(const DMA2D_HandleTypeDef *handle)
{
    return (handle == &DMA2D_GetDevice()->hal);
}

/* Public functions ----------------------------------------------------------*/

void DMA2D_RegisterTransferCompleteCallback(DMA2D_TransferCompleteCallback callback)
{
    s_completeCallback = callback;
    log_debug("DMA2D transfer complete callback registered");
}

void DMA2D_RegisterTransferErrorCallback(DMA2D_TransferErrorCallback callback)
{
    s_errorCallback = callback;
    log_debug("DMA2D transfer error callback registered");
}

void DMA2D_RegisterTransferProgressCallback(DMA2D_TransferProgressCallback callback)
{
    s_progressCallback = callback;
    log_debug("DMA2D transfer progress callback registered");
}

void DMA2D_ClearCallbacks(void)
{
    s_completeCallback = NULL;
    s_errorCallback = NULL;
    s_progressCallback = NULL;
}

/* HAL callback overrides ----------------------------------------------------*/

void HAL_DMA2D_TransferCompleteCallback(DMA2D_HandleTypeDef *hdma2d)
{
    if (!DMA2D_IsOwnHandle(hdma2d)) {
        return;
    }

    DMA2D_UpdateStatus(HAL_OK);
    log_debug("DMA2D transfer completed");

    if (s_completeCallback != NULL) {
        s_completeCallback(hdma2d);
    }
}

void HAL_DMA2D_TransferErrorCallback(DMA2D_HandleTypeDef *hdma2d)
{
    if (!DMA2D_IsOwnHandle(hdma2d)) {
        return;
    }

    DMA2D_UpdateStatus(HAL_ERROR);
    log_error("DMA2D transfer error occurred");

    if (s_errorCallback != NULL) {
        s_errorCallback(hdma2d);
    }
}

void HAL_DMA2D_TransferProgressCallback(DMA2D_HandleTypeDef *hdma2d, uint32_t progress)
{
    if (!DMA2D_IsOwnHandle(hdma2d)) {
        return;
    }

    log_debug("DMA2D transfer progress: %lu%%", (unsigned long)progress);

    if (s_progressCallback != NULL) {
        s_progressCallback(hdma2d, progress);
    }
}
