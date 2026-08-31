/**
 * @file ltdc_layer.c
 * @brief LTDC layer configuration and geometry
 */

#include "ltdc_layer.h"
#include "ltdc_core.h"
#include "ltdc_pixel.h"
#include "ltdc_sync.h"
#include "log.h"

/* Private types -------------------------------------------------------------*/

/** Blending factors that realise one blend mode. */
typedef struct {
    LTDC_BlendMode_t mode;
    uint32_t factor1;
    uint32_t factor2;
} LTDC_BlendEntry_t;

/* Private data --------------------------------------------------------------*/

static const LTDC_BlendEntry_t s_blendModes[] = {
    {LTDC_BLEND_CONSTANT_ALPHA, LTDC_BLENDING_FACTOR1_CA,     LTDC_BLENDING_FACTOR2_CA},
    {LTDC_BLEND_PIXEL_ALPHA,    LTDC_BLENDING_FACTOR1_PAxCA,  LTDC_BLENDING_FACTOR2_PAxCA},
    {LTDC_BLEND_NO_BLENDING,    LTDC_BLENDING_FACTOR1_CA,     LTDC_BLENDING_FACTOR2_CA},
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Translate a driver layer configuration into the HAL structure
 * @param config Driver layer configuration
 * @param layerCfg Destination HAL structure
 */
static void LTDC_BuildLayerCfg(const LTDC_LayerConfig_t *config, LTDC_LayerCfgTypeDef *layerCfg)
{
    layerCfg->WindowX0 = config->windowX0;
    layerCfg->WindowX1 = config->windowX1;
    layerCfg->WindowY0 = config->windowY0;
    layerCfg->WindowY1 = config->windowY1;
    layerCfg->PixelFormat = LTDC_PixelFormatToHAL(config->pixelFormat);
    layerCfg->Alpha = config->alpha;
    layerCfg->Alpha0 = config->alpha0;
    layerCfg->FBStartAdress = config->framebufferAddress;
    layerCfg->ImageWidth = config->imageWidth;
    layerCfg->ImageHeight = config->imageHeight;

    /* Unknown modes fall back to constant alpha. */
    layerCfg->BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    layerCfg->BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
    for (uint32_t i = 0; i < (sizeof(s_blendModes) / sizeof(s_blendModes[0])); i++) {
        if (s_blendModes[i].mode == config->blendMode) {
            layerCfg->BlendingFactor1 = s_blendModes[i].factor1;
            layerCfg->BlendingFactor2 = s_blendModes[i].factor2;
            break;
        }
    }

    layerCfg->Backcolor.Blue  = (uint8_t)(config->backgroundColor & 0xFFU);
    layerCfg->Backcolor.Green = (uint8_t)((config->backgroundColor >> 8) & 0xFFU);
    layerCfg->Backcolor.Red   = (uint8_t)((config->backgroundColor >> 16) & 0xFFU);
}

/**
 * @brief Validate the driver and layer index in one step
 * @param driver Driver record
 * @param layer Layer number
 * @return HAL_StatusTypeDef HAL_OK when both are valid
 */
static HAL_StatusTypeDef LTDC_CheckLayerAccess(LTDC_Driver_t *driver, uint8_t layer)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }
    return HAL_OK;
}

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef LTDC_ConfigureLayer(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *config)
{
    log_debug("LTDC: Configuring layer %d", layer);

    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK || config == NULL) {
        log_error("LTDC: Invalid parameters for layer %d config", layer);
        if (driver != NULL) {
            driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        }
        return HAL_ERROR;
    }

    /* Packed 24-bit pixels break the LTDC/DMA 32-bit burst alignment; use ARGB8888 instead. */
    if (config->pixelFormat == LTDC_PIXEL_FORMAT_RGB888_ENUM) {
        driver->errorCode = LTDC_ERROR_UNSUPPORTED_FORMAT;
        return HAL_ERROR;
    }

    LTDC_LayerCfgTypeDef layerCfg = {0};
    LTDC_BuildLayerCfg(config, &layerCfg);

    HAL_StatusTypeDef status = HAL_LTDC_ConfigLayer(driver->hltdc, &layerCfg, layer);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
        return status;
    }

    driver->layers[layer] = *config;

    log_debug("LTDC: Layer %d configured successfully", layer);
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_EnableLayer(LTDC_Driver_t *driver, uint8_t layer)
{
    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK) {
        return HAL_ERROR;
    }

    __HAL_LTDC_LAYER_ENABLE(driver->hltdc, layer);

    HAL_StatusTypeDef status = LTDC_RequestReload(driver, LTDC_SRCR_VBR);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
        return status;
    }

    driver->layers[layer].enabled = true;
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_DisableLayer(LTDC_Driver_t *driver, uint8_t layer)
{
    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK) {
        return HAL_ERROR;
    }

    __HAL_LTDC_LAYER_DISABLE(driver->hltdc, layer);
    __HAL_LTDC_RELOAD_CONFIG(driver->hltdc);

    driver->layers[layer].enabled = false;
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_SetActiveLayer(LTDC_Driver_t *driver, uint8_t layer)
{
    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK) {
        return HAL_ERROR;
    }

    driver->activeLayer = layer;
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_SetLayerAlpha(LTDC_Driver_t *driver, uint8_t layer, uint8_t alpha)
{
    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_LTDC_SetAlpha(driver->hltdc, alpha, layer);
    if (status == HAL_OK) {
        driver->layers[layer].alpha = alpha;
    }

    return status;
}

HAL_StatusTypeDef LTDC_SetWindowPosition_NoReload(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint8_t layer)
{
    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LTDC_ValidateCoordinates(x, y) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_LTDC_SetWindowPosition_NoReload(driver->hltdc, x, y, layer);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
        return status;
    }

    /* windowX1/windowY1 are inclusive, so the size is one more than the difference. */
    LTDC_LayerConfig_t *cached = &driver->layers[layer];
    uint16_t width = (cached->windowX1 - cached->windowX0) + 1;
    uint16_t height = (cached->windowY1 - cached->windowY0) + 1;
    cached->windowX0 = x;
    cached->windowY0 = y;
    cached->windowX1 = x + width - 1;
    cached->windowY1 = y + height - 1;

    return HAL_OK;
}

HAL_StatusTypeDef LTDC_SetLayerPosition(LTDC_Driver_t *driver, uint8_t layer, uint16_t x, uint16_t y)
{
    HAL_StatusTypeDef status = LTDC_SetWindowPosition_NoReload(driver, x, y, layer);
    if (status != HAL_OK) {
        return status;
    }

    return LTDC_RequestReload(driver, LTDC_SRCR_VBR);
}

HAL_StatusTypeDef LTDC_SetLayerWindow(LTDC_Driver_t *driver, uint8_t layer, LTDC_Rect_t *window)
{
    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK) {
        return HAL_ERROR;
    }
    if (LTDC_ValidateRect(window) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Keep every other layer property; only geometry changes. */
    LTDC_LayerConfig_t config = driver->layers[layer];
    config.windowX0 = window->x;
    config.windowY0 = window->y;
    config.windowX1 = window->x + window->width - 1;    /* inclusive */
    config.windowY1 = window->y + window->height - 1;   /* inclusive */
    config.imageWidth = window->width;
    config.imageHeight = window->height;

    LTDC_LayerCfgTypeDef layerCfg = {0};
    LTDC_BuildLayerCfg(&config, &layerCfg);

    HAL_StatusTypeDef status = HAL_LTDC_ConfigLayer(driver->hltdc, &layerCfg, layer);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
        return status;
    }

    driver->layers[layer] = config;
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_GetLayerInfo(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *info)
{
    if (LTDC_CheckLayerAccess(driver, layer) != HAL_OK || info == NULL) {
        if (driver != NULL) {
            driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        }
        return HAL_ERROR;
    }

    *info = driver->layers[layer];
    return HAL_OK;
}

bool LTDC_IsLayerEnabled(LTDC_Driver_t *driver, uint8_t layer)
{
    if (driver == NULL || layer >= LTDC_MAX_LAYERS) {
        return false;
    }
    return driver->layers[layer].enabled;
}
