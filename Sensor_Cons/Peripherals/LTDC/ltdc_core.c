/**
 * @file ltdc_core.c
 * @brief LTDC driver lifecycle, display configuration and shared validation
 */

#include "ltdc_core.h"
#include "ltdc_events.h"
#include "ltdc_layer.h"
#include "log.h"
#include <string.h>

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Split a 0x00RRGGBB colour into the HAL background colour fields
 * @param color Packed colour
 * @param backcolor Destination HAL structure
 */
static void LTDC_UnpackBackcolor(uint32_t color, LTDC_ColorTypeDef *backcolor) {
    backcolor->Blue = (uint8_t)(color & LTDC_CHANNEL_MASK);
    backcolor->Green = (uint8_t)((color >> LTDC_GREEN_SHIFT) & LTDC_CHANNEL_MASK);
    backcolor->Red = (uint8_t)((color >> LTDC_RED_SHIFT) & LTDC_CHANNEL_MASK);
}

/**
 * @brief Translate true porch widths into the accumulated register values
 * @param config Display configuration
 * @param init Destination HAL init structure
 */
static void LTDC_BuildTimings(const LTDC_DisplayConfig_t *config, LTDC_InitTypeDef *init) {
    init->HSPolarity = config->hsyncActiveLow ? LTDC_HSPOLARITY_AL : LTDC_HSPOLARITY_AH;
    init->VSPolarity = config->vsyncActiveLow ? LTDC_VSPOLARITY_AL : LTDC_VSPOLARITY_AH;
    init->DEPolarity = config->dataEnableActiveLow ? LTDC_DEPOLARITY_AL : LTDC_DEPOLARITY_AH;
    init->PCPolarity = config->pixelClockInverted ? LTDC_PCPOLARITY_IIPC : LTDC_PCPOLARITY_IPC;

    /* LTDC timing registers hold accumulated widths minus one. */
    init->HorizontalSync = (uint32_t)config->hsyncWidth - 1U;
    init->VerticalSync = (uint32_t)config->vsyncHeight - 1U;
    init->AccumulatedHBP = (uint32_t)config->hsyncWidth + config->hbp - 1U;
    init->AccumulatedVBP = (uint32_t)config->vsyncHeight + config->vbp - 1U;
    init->AccumulatedActiveW = init->AccumulatedHBP + config->width;
    init->AccumulatedActiveH = init->AccumulatedVBP + config->height;
    init->TotalWidth = init->AccumulatedActiveW + config->hfp;
    init->TotalHeigh = init->AccumulatedActiveH + config->vfp;

    LTDC_UnpackBackcolor(config->backgroundColor, &init->Backcolor);
}

/**
 * @brief Reject a display configuration the controller cannot encode
 * @param config Display configuration
 * @return HAL_OK when every mandatory field is non-zero
 */
static HAL_StatusTypeDef LTDC_ValidateDisplayConfig(const LTDC_DisplayConfig_t *config) {
    /* Sync widths are stored minus one, so zero would underflow the register. */
    if (config == NULL || config->width == 0U || config->height == 0U || config->hsyncWidth == 0U ||
        config->vsyncHeight == 0U) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief Give every layer without a framebuffer a full-screen window
 * @param driver Driver record whose displayConfig is already set
 */
static void LTDC_DefaultLayerWindows(LTDC_Driver_t *driver) {
    for (uint8_t i = 0; i < LTDC_MAX_LAYERS; i++) {
        LTDC_LayerConfig_t *layer = &driver->layers[i];
        if (layer->framebufferAddress != 0U) {
            continue; /* Already configured by the application */
        }
        layer->windowX0 = 0;
        layer->windowY0 = 0;
        layer->windowX1 = driver->displayConfig.width - 1; /* inclusive end coordinate */
        layer->windowY1 = driver->displayConfig.height - 1;
        layer->imageWidth = driver->displayConfig.width;
        layer->imageHeight = driver->displayConfig.height;
    }
}

/* Lifecycle -----------------------------------------------------------------*/

HAL_StatusTypeDef LTDC_Driver_Init(LTDC_Driver_t *driver, LTDC_HandleTypeDef *hltdc_handle) {
    log_debug("LTDC: Initializing driver");

    if (driver == NULL || hltdc_handle == NULL) {
        log_error("LTDC: Invalid parameters for init");
        return HAL_ERROR;
    }

    memset(driver, 0, sizeof(LTDC_Driver_t));
    driver->hltdc = hltdc_handle;
    driver->errorCode = LTDC_ERROR_NONE;

    /* Window geometry stays zero until LTDC_ConfigureDisplay() knows the panel. */
    for (uint8_t i = 0; i < LTDC_MAX_LAYERS; i++) {
        LTDC_LayerConfig_t *layer = &driver->layers[i];
        layer->pixelFormat = LTDC_PIXEL_FORMAT_RGB565_ENUM;
        layer->alpha = LTDC_MAX_ALPHA;
        layer->alpha0 = 0;
        layer->blendMode = LTDC_BLEND_CONSTANT_ALPHA;
        layer->backgroundColor = LTDC_COLOR_BLACK;
        layer->enabled = false;
        layer->framebufferAddress = 0;
    }

    driver->activeLayer = 0;
    driver->initialized = true;

    LTDC_Events_Attach(driver);

    log_debug("LTDC: Driver initialized successfully");
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_Driver_DeInit(LTDC_Driver_t *driver) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Keep tearing down every layer even if one refuses, but remember the
       first failure so the caller is not told the driver shut down cleanly. */
    HAL_StatusTypeDef status = HAL_OK;

    for (uint8_t i = 0; i < LTDC_MAX_LAYERS; i++) {
        if (LTDC_DisableLayer(driver, i) != HAL_OK && status == HAL_OK) {
            status = HAL_ERROR;
        }
    }

    if (HAL_LTDC_DeInit(driver->hltdc) != HAL_OK) {
        status = HAL_ERROR;
    }

    driver->initialized = false;
    driver->errorCode = LTDC_ERROR_NONE;
    LTDC_Events_Detach(driver);

    return status;
}

HAL_StatusTypeDef LTDC_ConfigureDisplay(LTDC_Driver_t *driver, const LTDC_DisplayConfig_t *config) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateDisplayConfig(config) != HAL_OK) {
        log_error("LTDC: Invalid driver or config for display setup");
        if (driver != NULL) {
            driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        }
        return HAL_ERROR;
    }

    log_debug("LTDC: Configuring display %dx%d", config->width, config->height);

    LTDC_HandleTypeDef *hltdc = driver->hltdc;
    hltdc->Instance = LTDC; /* The STM32F4 has exactly one controller */
    LTDC_BuildTimings(config, &hltdc->Init);

    if (hltdc->State == HAL_LTDC_STATE_RESET) {
        HAL_StatusTypeDef status = HAL_LTDC_Init(hltdc);
        if (status != HAL_OK) {
            driver->errorCode = LTDC_ERROR_INIT_FAILED;
            return status;
        }
    }

    driver->displayConfig = *config;
    LTDC_DefaultLayerWindows(driver);

    log_debug("LTDC: Display configured successfully");
    return HAL_OK;
}

/* Display control -----------------------------------------------------------*/

HAL_StatusTypeDef LTDC_SetBackgroundColor(LTDC_Driver_t *driver, uint32_t color) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_ColorTypeDef backcolor;
    LTDC_UnpackBackcolor(color, &backcolor);

    driver->hltdc->Instance->BCCR = ((uint32_t)backcolor.Red << LTDC_RED_SHIFT) |
                                    ((uint32_t)backcolor.Green << LTDC_GREEN_SHIFT) |
                                    (uint32_t)backcolor.Blue;
    __HAL_LTDC_RELOAD_CONFIG(driver->hltdc);

    driver->displayConfig.backgroundColor = color;
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_DisplayOn(LTDC_Driver_t *driver) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    __HAL_LTDC_ENABLE(driver->hltdc);
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_DisplayOff(LTDC_Driver_t *driver) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    __HAL_LTDC_DISABLE(driver->hltdc);
    return HAL_OK;
}

/* Error handling ------------------------------------------------------------*/

uint32_t LTDC_GetError(LTDC_Driver_t *driver) {
    if (driver == NULL) {
        return LTDC_ERROR_INVALID_PARAM;
    }
    return driver->errorCode;
}

HAL_StatusTypeDef LTDC_ClearError(LTDC_Driver_t *driver) {
    if (driver == NULL) {
        return HAL_ERROR;
    }
    driver->errorCode = LTDC_ERROR_NONE;
    return HAL_OK;
}

/* Shared validation ---------------------------------------------------------*/

HAL_StatusTypeDef LTDC_ValidateDriver(LTDC_Driver_t *driver) {
    if (driver == NULL || !driver->initialized || driver->hltdc == NULL) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_ValidateLayer(uint8_t layer) {
    return (layer < LTDC_MAX_LAYERS) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef LTDC_ValidateCoordinates(const LTDC_Driver_t *driver, uint16_t xCoord,
                                           uint16_t yCoord) {
    /* An unconfigured display has zero size, so nothing is on screen yet. */
    if (driver == NULL || xCoord >= driver->displayConfig.width ||
        yCoord >= driver->displayConfig.height) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_ValidateRect(const LTDC_Driver_t *driver, const LTDC_Rect_t *rect) {
    if (driver == NULL || rect == NULL) {
        return HAL_ERROR;
    }

    const uint32_t width = driver->displayConfig.width;
    const uint32_t height = driver->displayConfig.height;

    if (rect->x >= width || rect->y >= height || ((uint32_t)rect->x + rect->width) > width ||
        ((uint32_t)rect->y + rect->height) > height) {
        return HAL_ERROR;
    }
    return HAL_OK;
}
