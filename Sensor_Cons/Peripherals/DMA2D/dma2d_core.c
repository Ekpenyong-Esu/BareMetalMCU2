/**
 * @file dma2d_core.c
 * @brief DMA2D lifecycle, layer configuration and driver state
 */

#include "dma2d_core.h"
#include "dma2d_color.h"
#include "dma2d_events.h"
#include "dma2d_validate.h"
#include "log.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/

/** The MCU has exactly one Chrom-Art unit, hence one device record. */
static DMA2D_Device s_device;

/* Public functions ----------------------------------------------------------*/

DMA2D_Device* DMA2D_GetDevice(void)
{
    return &s_device;
}

bool DMA2D_IsInitialized(void)
{
    return s_device.status.initialized;
}

bool DMA2D_IsBusy(void)
{
    return (s_device.hal.State == HAL_DMA2D_STATE_BUSY);
}

void DMA2D_UpdateStatus(HAL_StatusTypeDef result)
{
    s_device.status.state = s_device.hal.State;
    s_device.status.last_error = result;

    if (result == HAL_OK) {
        s_device.status.transfer_count++;
    } else if (result != HAL_BUSY) {
        s_device.status.error_count++;
    }
}

HAL_StatusTypeDef DMA2D_GetStatus(DMA2D_Status *status)
{
    if (status == NULL) {
        log_error("DMA2D status pointer is NULL");
        return HAL_ERROR;
    }

    *status = s_device.status;
    return HAL_OK;
}

HAL_StatusTypeDef DMA2D_Init(const DMA2D_Config *config)
{
    log_debug("Initializing DMA2D peripheral");

    HAL_StatusTypeDef result = DMA2D_ValidateConfig(config);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    __HAL_RCC_DMA2D_CLK_ENABLE();

    s_device.hal.Instance = DMA2D;
    s_device.hal.Init.Mode = config->mode;
    s_device.hal.Init.ColorMode = config->color_mode;
    s_device.hal.Init.OutputOffset = config->output_offset;

    result = HAL_DMA2D_Init(&s_device.hal);
    if (result != HAL_OK) {
        log_error("HAL_DMA2D_Init failed: %s", DMA2D_GetErrorString(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Register-to-memory transfers take their colour from the output layer. */
    if (config->mode == DMA2D_MODE_R2M) {
        HAL_DMA2D_ConfigDeadTime(&s_device.hal, 0);

        s_device.hal.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = config->color_mode;
        s_device.hal.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha =
            DMA2D_MakeColor((uint8_t)config->red_value,
                            (uint8_t)config->green_value,
                            (uint8_t)config->blue_value,
                            (uint8_t)config->alpha_value);
        s_device.hal.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = config->output_offset;

        /* HAL_DMA2D_ConfigLayer copies LayerCfg into the registers, so it has to
           run after the fields are filled in. */
        HAL_DMA2D_ConfigLayer(&s_device.hal, DMA2D_FOREGROUND_LAYER);
    }

    s_device.status.initialized = true;
    DMA2D_UpdateStatus(result);

    log_info("DMA2D initialized successfully");
    return HAL_OK;
}

HAL_StatusTypeDef DMA2D_DeInit(void)
{
    log_debug("Deinitializing DMA2D peripheral");

    if (!s_device.status.initialized) {
        log_warning("DMA2D not initialized");
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        HAL_StatusTypeDef aborted = HAL_DMA2D_Abort(&s_device.hal);
        if (aborted != HAL_OK) {
            log_warning("Failed to abort ongoing DMA2D transfer: %s",
                        DMA2D_GetErrorString(aborted));
        }
    }

    HAL_StatusTypeDef result = HAL_DMA2D_DeInit(&s_device.hal);
    if (result != HAL_OK) {
        log_error("HAL_DMA2D_DeInit failed: %s", DMA2D_GetErrorString(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    __HAL_RCC_DMA2D_CLK_DISABLE();

    memset(&s_device.status, 0, sizeof(s_device.status));
    s_device.status.state = HAL_DMA2D_STATE_RESET;

    DMA2D_ClearCallbacks();

    log_info("DMA2D deinitialized successfully");
    return HAL_OK;
}

HAL_StatusTypeDef DMA2D_ConfigLayer(uint32_t layer, const DMA2D_LayerConfig *layer_config)
{
    log_debug("Configuring DMA2D layer %lu", (unsigned long)layer);

    HAL_StatusTypeDef result = DMA2D_ValidateLayerConfig(layer_config);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    result = DMA2D_ValidateLayer(layer);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (!s_device.status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    /* DMA2D_BACKGROUND_LAYER and DMA2D_FOREGROUND_LAYER are the LayerCfg indices
       themselves (0 and 1), so no remapping is needed. */
    s_device.hal.LayerCfg[layer].InputColorMode = layer_config->input_color_mode;
    s_device.hal.LayerCfg[layer].AlphaMode = layer_config->input_alpha_mode;
    s_device.hal.LayerCfg[layer].InputAlpha = layer_config->input_alpha;
    s_device.hal.LayerCfg[layer].InputOffset = layer_config->input_offset;

    result = HAL_DMA2D_ConfigLayer(&s_device.hal, layer);
    if (result != HAL_OK) {
        log_error("HAL_DMA2D_ConfigLayer failed: %s", DMA2D_GetErrorString(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    log_debug("DMA2D layer %lu configured successfully", (unsigned long)layer);
    return HAL_OK;
}

HAL_StatusTypeDef DMA2D_PollForTransfer(uint32_t timeout)
{
    log_debug("Polling for DMA2D transfer completion, timeout=%lu", (unsigned long)timeout);

    if (!s_device.status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    HAL_StatusTypeDef result = HAL_DMA2D_PollForTransfer(&s_device.hal, timeout);
    if (result != HAL_OK) {
        log_error("DMA2D poll failed: %s", DMA2D_GetErrorString(result));
    }

    DMA2D_UpdateStatus(result);
    return result;
}

HAL_StatusTypeDef DMA2D_Abort(void)
{
    log_debug("Aborting DMA2D transfer");

    if (!s_device.status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    HAL_StatusTypeDef result = HAL_DMA2D_Abort(&s_device.hal);
    if (result != HAL_OK) {
        log_error("DMA2D abort failed: %s", DMA2D_GetErrorString(result));
    } else {
        log_info("DMA2D transfer aborted successfully");
    }

    DMA2D_UpdateStatus(result);
    return result;
}

HAL_StatusTypeDef DMA2D_EnableLCDMode(void)
{
    log_info("Enabling DMA2D LCD mode");

    if (!s_device.status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    s_device.hal.Init.OutputOffset = 0;
    s_device.hal.Init.ColorMode = DMA2D_OUTPUT_RGB565;  /* Common LCD format */

    HAL_StatusTypeDef result = HAL_DMA2D_Init(&s_device.hal);
    if (result != HAL_OK) {
        log_error("Failed to configure DMA2D for LCD mode: %s", DMA2D_GetErrorString(result));
        return result;
    }

    log_info("DMA2D LCD mode enabled");
    return HAL_OK;
}

void DMA2D_ISR_Dispatch(void)
{
    HAL_DMA2D_IRQHandler(&s_device.hal);
}
