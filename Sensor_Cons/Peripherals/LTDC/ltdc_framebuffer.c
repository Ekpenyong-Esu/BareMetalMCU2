/**
 * @file ltdc_framebuffer.c
 * @brief Framebuffer ownership and direct pixel access
 */

#include "ltdc_framebuffer.h"
#include "ltdc_core.h"
#include "ltdc_pixel.h"
#include "ltdc_sync.h"
#include <string.h>

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Resolve a layer that owns a framebuffer
 * @param driver Driver record
 * @param layer Layer number
 * @return LTDC_LayerConfig_t* Layer configuration, or NULL when unusable
 */
static LTDC_LayerConfig_t *LTDC_ResolveLayerBuffer(LTDC_Driver_t *driver, uint8_t layer) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return NULL;
    }
    if (LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return NULL;
    }
    if (driver->layers[layer].framebufferAddress == 0) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return NULL;
    }
    return &driver->layers[layer];
}

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef LTDC_SetFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t address) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LTDC_ValidateLayer(layer) != HAL_OK || address == 0) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_LTDC_SetAddress(driver->hltdc, address, layer);
    if (status != HAL_OK) {
        return status;
    }

    /* Swap at the vertical blanking interval so the change never tears. */
    if (LTDC_RequestReload(driver, LTDC_SRCR_VBR) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return HAL_ERROR;
    }

    if (LTDC_WaitForReload(driver, LTDC_RELOAD_TIMEOUT_MS) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return HAL_TIMEOUT;
    }

    driver->layers[layer].framebufferAddress = address;
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_ClearFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color) {
    LTDC_LayerConfig_t *cfg = LTDC_ResolveLayerBuffer(driver, layer);
    if (cfg == NULL) {
        return HAL_ERROR;
    }

    LTDC_FillPixels((uint8_t *)(uintptr_t)cfg->framebufferAddress,
                    (uint32_t)cfg->imageWidth * cfg->imageHeight, color, cfg->pixelFormat);

    /* The pixels are already written; without the reload the panel keeps
       scanning the old buffer, so a failure here is visible on screen. */
    if (LTDC_RequestReload(driver, LTDC_SRCR_VBR) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef LTDC_CopyFramebuffer(LTDC_Driver_t *driver, uint8_t srcLayer, uint8_t dstLayer) {
    LTDC_LayerConfig_t *src = LTDC_ResolveLayerBuffer(driver, srcLayer);
    LTDC_LayerConfig_t *dst = LTDC_ResolveLayerBuffer(driver, dstLayer);
    if (src == NULL || dst == NULL) {
        return HAL_ERROR;
    }

    uint32_t srcPixels = (uint32_t)src->imageWidth * src->imageHeight;
    uint32_t dstPixels = (uint32_t)dst->imageWidth * dst->imageHeight;
    uint32_t copyPixels = (srcPixels < dstPixels) ? srcPixels : dstPixels;

    memcpy((void *)(uintptr_t)dst->framebufferAddress, (void *)(uintptr_t)src->framebufferAddress,
           copyPixels * LTDC_PixelSize(src->pixelFormat));

    return HAL_OK;
}

HAL_StatusTypeDef LTDC_DrawPixel(LTDC_Driver_t *driver, uint16_t posX, uint16_t posY,
                                 uint32_t color) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LTDC_ValidateCoordinates(driver, posX, posY) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    LTDC_LayerConfig_t *cfg = LTDC_ResolveLayerBuffer(driver, driver->activeLayer);
    if (cfg == NULL) {
        return HAL_ERROR;
    }

    if (posX < cfg->windowX0 || posX > cfg->windowX1 || posY < cfg->windowY0 ||
        posY > cfg->windowY1) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    uint32_t index =
        ((uint32_t)(posY - cfg->windowY0) * cfg->imageWidth) + (uint32_t)(posX - cfg->windowX0);
    LTDC_WritePixel((uint8_t *)(uintptr_t)cfg->framebufferAddress, index, color, cfg->pixelFormat);

    return HAL_OK;
}
