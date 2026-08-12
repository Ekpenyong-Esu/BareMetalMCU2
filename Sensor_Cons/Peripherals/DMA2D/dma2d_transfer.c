/**
 * @file dma2d_transfer.c
 * @brief DMA2D transfer operations (polling and interrupt driven)
 */

#include "dma2d_transfer.h"
#include "dma2d_core.h"
#include "dma2d_color.h"
#include "dma2d_validate.h"
#include "log.h"

/* Private types -------------------------------------------------------------*/

/** How the caller wants to observe completion. */
typedef enum {
    DMA2D_WAIT_POLLING = 0,   /**< Block until the transfer finishes */
    DMA2D_WAIT_INTERRUPT      /**< Return as soon as the transfer is armed */
} DMA2D_WaitMode;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Common precondition check for every transfer entry point
 * @return HAL_OK when a transfer may be started
 */
static HAL_StatusTypeDef DMA2D_ReadyToStart(void)
{
    if (!DMA2D_IsInitialized()) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        log_warning("DMA2D is busy");
        return HAL_BUSY;
    }

    return HAL_OK;
}

/**
 * @brief Arm the peripheral for the configured mode
 * @details This is the only place that maps an operating mode onto a HAL start
 *          function, which is why the polling and interrupt paths cannot drift
 *          apart.
 * @param wait   Polling or interrupt completion
 * @param pSrc   Source buffer, ignored in register-to-memory mode
 * @param pDst   Destination buffer
 * @param width  Transfer width in pixels
 * @param height Transfer height in pixels
 * @return HAL status of the start call
 */
static HAL_StatusTypeDef DMA2D_LaunchConfiguredMode(DMA2D_WaitMode wait,
                                                    const uint32_t *pSrc, uint32_t *pDst,
                                                    uint32_t width, uint32_t height)
{
    DMA2D_HandleTypeDef *hal = &DMA2D_GetDevice()->hal;
    const bool useInterrupt = (wait == DMA2D_WAIT_INTERRUPT);

    switch (hal->Init.Mode) {
        case DMA2D_R2M:
            return useInterrupt
                ? HAL_DMA2D_Start_IT(hal, 0, (uint32_t)pDst, width, height)
                : HAL_DMA2D_Start(hal, 0, (uint32_t)pDst, width, height);

        case DMA2D_M2M:
        case DMA2D_M2M_PFC:
            return useInterrupt
                ? HAL_DMA2D_Start_IT(hal, (uint32_t)pSrc, (uint32_t)pDst, width, height)
                : HAL_DMA2D_Start(hal, (uint32_t)pSrc, (uint32_t)pDst, width, height);

        case DMA2D_M2M_BLEND:
            return useInterrupt
                ? HAL_DMA2D_BlendingStart_IT(hal, (uint32_t)pSrc, 0, (uint32_t)pDst, width, height)
                : HAL_DMA2D_BlendingStart(hal, (uint32_t)pSrc, 0, (uint32_t)pDst, width, height);

        default:
            log_error("Unsupported DMA2D mode: %lu", (unsigned long)hal->Init.Mode);
            return HAL_ERROR;
    }
}

/**
 * @brief Point both input layers at plain ARGB8888 data with no line offset
 * @note  HAL_DMA2D_ConfigLayer copies LayerCfg into the registers, so the
 *        fields have to be written before it is called.
 */
static void DMA2D_ConfigureBlendLayers(void)
{
    DMA2D_HandleTypeDef *hal = &DMA2D_GetDevice()->hal;

    for (uint32_t layer = DMA2D_BACKGROUND_LAYER; layer <= DMA2D_FOREGROUND_LAYER; layer++) {
        hal->LayerCfg[layer].InputColorMode = DMA2D_INPUT_ARGB8888;
        hal->LayerCfg[layer].InputOffset = 0;
        HAL_DMA2D_ConfigLayer(hal, layer);
    }
}

/**
 * @brief Optionally wait for the transfer, then fold the result into the counters
 * @param wait    Completion mode chosen by the caller
 * @param result  Status returned by the start call
 * @param context Short label used in the failure log
 * @return Final HAL status of the operation
 */
static HAL_StatusTypeDef DMA2D_FinishTransfer(DMA2D_WaitMode wait,
                                              HAL_StatusTypeDef result,
                                              const char *context)
{
    if (result != HAL_OK) {
        const char *errStr = DMA2D_GetErrorString(result);
        log_error("DMA2D %s start failed: %s", context, errStr);
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (wait == DMA2D_WAIT_POLLING) {
        DMA2D_Device *device = DMA2D_GetDevice();
        result = HAL_DMA2D_PollForTransfer(&device->hal, DMA2D_DEFAULT_TIMEOUT);
        if (result != HAL_OK) {
            const char *pollErr = DMA2D_GetErrorString(result);
            log_error("DMA2D %s poll failed: %s", context, pollErr);
            DMA2D_UpdateStatus(result);
            return result;
        }
    }

    DMA2D_UpdateStatus(result);
    return HAL_OK;
}

/**
 * @brief Shared implementation of DMA2D_StartTransfer and DMA2D_StartTransfer_IT
 */
static HAL_StatusTypeDef DMA2D_Transfer(DMA2D_WaitMode wait, const uint32_t *pSrc,
                                        uint32_t *pDst, uint32_t width, uint32_t height)
{
    uint32_t halMode = DMA2D_GetDevice()->hal.Init.Mode;
    HAL_StatusTypeDef result = DMA2D_ValidateTransfer(halMode,
                                                      pSrc, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    result = DMA2D_ReadyToStart();
    if (result != HAL_OK) {
        return result;
    }

    return DMA2D_FinishTransfer(wait,
                                DMA2D_LaunchConfiguredMode(wait, pSrc, pDst, width, height),
                                "transfer");
}

/**
 * @brief Shared implementation of DMA2D_StartFill and DMA2D_StartFill_IT
 */
static HAL_StatusTypeDef DMA2D_Fill(DMA2D_WaitMode wait, uint32_t color, uint32_t *pDst,
                                    uint32_t width, uint32_t height)
{
    /* A fill never reads memory, so it is validated as a register-to-memory op. */
    HAL_StatusTypeDef result = DMA2D_ValidateTransfer(DMA2D_R2M, NULL, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    result = DMA2D_ReadyToStart();
    if (result != HAL_OK) {
        return result;
    }

    DMA2D_HandleTypeDef *hal = &DMA2D_GetDevice()->hal;
    result = (wait == DMA2D_WAIT_INTERRUPT)
        ? HAL_DMA2D_Start_IT(hal, color, (uint32_t)pDst, width, height)
        : HAL_DMA2D_Start(hal, color, (uint32_t)pDst, width, height);

    return DMA2D_FinishTransfer(wait, result, "fill");
}

/**
 * @brief Shared implementation of DMA2D_StartBlending and DMA2D_StartBlending_IT
 */
static HAL_StatusTypeDef DMA2D_Blend(DMA2D_WaitMode wait, const uint32_t *pSrc1,
                                     const uint32_t *pSrc2, uint32_t *pDst,
                                     uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef result = DMA2D_ValidateTransfer(DMA2D_M2M_BLEND, pSrc1, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (pSrc2 == NULL) {
        log_error("DMA2D blending source 2 buffer is NULL");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    result = DMA2D_ReadyToStart();
    if (result != HAL_OK) {
        return result;
    }

    DMA2D_ConfigureBlendLayers();

    DMA2D_HandleTypeDef *hal = &DMA2D_GetDevice()->hal;
    result = (wait == DMA2D_WAIT_INTERRUPT)
        ? HAL_DMA2D_BlendingStart_IT(hal, (uint32_t)pSrc1, (uint32_t)pSrc2,
                                     (uint32_t)pDst, width, height)
        : HAL_DMA2D_BlendingStart(hal, (uint32_t)pSrc1, (uint32_t)pSrc2,
                                  (uint32_t)pDst, width, height);

    return DMA2D_FinishTransfer(wait, result, "blending");
}

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef DMA2D_StartTransfer(const uint32_t *pSrc, uint32_t *pDst,
                                      uint32_t width, uint32_t height)
{
    log_debug("Starting DMA2D transfer: %lux%lu", (unsigned long)width, (unsigned long)height);
    return DMA2D_Transfer(DMA2D_WAIT_POLLING, pSrc, pDst, width, height);
}

HAL_StatusTypeDef DMA2D_StartTransfer_IT(const uint32_t *pSrc, uint32_t *pDst,
                                         uint32_t width, uint32_t height)
{
    log_debug("Starting DMA2D transfer (interrupt): %lux%lu",
              (unsigned long)width, (unsigned long)height);
    return DMA2D_Transfer(DMA2D_WAIT_INTERRUPT, pSrc, pDst, width, height);
}

HAL_StatusTypeDef DMA2D_StartFill(uint32_t color, uint32_t *pDst,
                                  uint32_t width, uint32_t height)
{
    log_debug("Starting DMA2D fill: color=0x%08lx, size=%lux%lu",
              (unsigned long)color, (unsigned long)width, (unsigned long)height);
    return DMA2D_Fill(DMA2D_WAIT_POLLING, color, pDst, width, height);
}

HAL_StatusTypeDef DMA2D_StartFill_IT(uint32_t color, uint32_t *pDst,
                                     uint32_t width, uint32_t height)
{
    log_debug("Starting DMA2D fill (interrupt): color=0x%08lx, size=%lux%lu",
              (unsigned long)color, (unsigned long)width, (unsigned long)height);
    return DMA2D_Fill(DMA2D_WAIT_INTERRUPT, color, pDst, width, height);
}

HAL_StatusTypeDef DMA2D_StartBlending(const uint32_t *pSrc1, const uint32_t *pSrc2,
                                      uint32_t *pDst, uint32_t width, uint32_t height)
{
    log_debug("Starting DMA2D blending: size=%lux%lu",
              (unsigned long)width, (unsigned long)height);
    return DMA2D_Blend(DMA2D_WAIT_POLLING, pSrc1, pSrc2, pDst, width, height);
}

HAL_StatusTypeDef DMA2D_StartBlending_IT(const uint32_t *pSrc1, const uint32_t *pSrc2,
                                         uint32_t *pDst, uint32_t width, uint32_t height)
{
    log_debug("Starting DMA2D blending (interrupt): size=%lux%lu",
              (unsigned long)width, (unsigned long)height);
    return DMA2D_Blend(DMA2D_WAIT_INTERRUPT, pSrc1, pSrc2, pDst, width, height);
}

HAL_StatusTypeDef DMA2D_SelfTest(void)
{
    log_info("Starting DMA2D self-test");

    if (!DMA2D_IsInitialized()) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    uint32_t test_buffer[16] = {0};
    HAL_StatusTypeDef result = DMA2D_StartFill(DMA2D_COLOR_RED, test_buffer, 4, 4);
    if (result != HAL_OK) {
        log_error("DMA2D self-test failed: %s", DMA2D_GetErrorString(result));
        return result;
    }

    for (size_t i = 0; i < (sizeof(test_buffer) / sizeof(test_buffer[0])); i++) {
        if (test_buffer[i] != DMA2D_COLOR_RED) {
            log_error("DMA2D self-test verification failed at index %u", (unsigned)i);
            return HAL_ERROR;
        }
    }

    log_info("DMA2D self-test passed");
    return HAL_OK;
}
