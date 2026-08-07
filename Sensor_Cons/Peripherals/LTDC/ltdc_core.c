/**
 * @file ltdc_core.c
 * @brief LTDC driver lifecycle, display configuration and shared validation
 */

#include "ltdc_core.h"
#include "ltdc_events.h"
#include "ltdc_layer.h"
#include "log.h"
#include <string.h>

/* Private data --------------------------------------------------------------*/

static LTDC_HandleTypeDef s_hltdc;   /*!< The one LTDC controller on this device */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Split a 0x00RRGGBB colour into the HAL background colour fields
 * @param color Packed colour
 * @param backcolor Destination HAL structure
 */
static void LTDC_UnpackBackcolor(uint32_t color, LTDC_ColorTypeDef *backcolor)
{
    backcolor->Blue  = (uint8_t)(color & 0xFFU);
    backcolor->Green = (uint8_t)((color >> 8) & 0xFFU);
    backcolor->Red   = (uint8_t)((color >> 16) & 0xFFU);
}

/* Handle and interrupt ------------------------------------------------------*/

LTDC_HandleTypeDef* LTDC_GetHandle(void)
{
    return &s_hltdc;
}

void LTDC_ISR_Dispatch(void)
{
    HAL_LTDC_IRQHandler(&s_hltdc);
}

/* Lifecycle -----------------------------------------------------------------*/

HAL_StatusTypeDef LTDC_Driver_Init(LTDC_Driver_t *driver, LTDC_HandleTypeDef *hltdc_handle)
{
    log_debug("LTDC: Initializing driver");

    if (driver == NULL || hltdc_handle == NULL) {
        log_error("LTDC: Invalid parameters for init");
        return HAL_ERROR;
    }

    memset(driver, 0, sizeof(LTDC_Driver_t));
    driver->hltdc = hltdc_handle;
    driver->errorCode = LTDC_ERROR_NONE;

    driver->displayConfig.width = LTDC_DISPLAY_WIDTH;
    driver->displayConfig.height = LTDC_DISPLAY_HEIGHT;
    driver->displayConfig.backgroundColor = LTDC_COLOR_BLACK;
    /* Polarities from the STM32F429I-DISC1 panel datasheet / CubeMX. */
    driver->displayConfig.hsyncActiveLow = true;
    driver->displayConfig.vsyncActiveLow = true;
    driver->displayConfig.dataEnableActiveLow = true;
    driver->displayConfig.pixelClockInverted = false;

    for (uint8_t i = 0; i < LTDC_MAX_LAYERS; i++) {
        LTDC_LayerConfig_t *layer = &driver->layers[i];
        layer->windowX0 = 0;
        layer->windowY0 = 0;
        layer->windowX1 = LTDC_DISPLAY_WIDTH - 1;   /* inclusive end coordinate */
        layer->windowY1 = LTDC_DISPLAY_HEIGHT - 1;
        layer->imageWidth = LTDC_DISPLAY_WIDTH;
        layer->imageHeight = LTDC_DISPLAY_HEIGHT;
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

HAL_StatusTypeDef LTDC_Driver_DeInit(LTDC_Driver_t *driver)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    for (uint8_t i = 0; i < LTDC_MAX_LAYERS; i++) {
        LTDC_DisableLayer(driver, i);
    }

    HAL_StatusTypeDef status = HAL_LTDC_DeInit(driver->hltdc);

    driver->initialized = false;
    driver->errorCode = LTDC_ERROR_NONE;
    LTDC_Events_Detach(driver);

    return status;
}

HAL_StatusTypeDef LTDC_ConfigureDisplay(LTDC_Driver_t *driver, LTDC_DisplayConfig_t *config)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK || config == NULL) {
        log_error("LTDC: Invalid driver or config for display setup");
        if (driver != NULL) {
            driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        }
        return HAL_ERROR;
    }

    log_debug("LTDC: Configuring display %dx%d", config->width, config->height);

    LTDC_HandleTypeDef *hltdc = driver->hltdc;
    hltdc->Instance = LTDC;

    hltdc->Init.HSPolarity = config->hsyncActiveLow ? LTDC_HSPOLARITY_AL : LTDC_HSPOLARITY_AH;
    hltdc->Init.VSPolarity = config->vsyncActiveLow ? LTDC_VSPOLARITY_AL : LTDC_VSPOLARITY_AH;
    hltdc->Init.DEPolarity = config->dataEnableActiveLow ? LTDC_DEPOLARITY_AL : LTDC_DEPOLARITY_AH;
    hltdc->Init.PCPolarity = config->pixelClockInverted ? LTDC_PCPOLARITY_IIPC : LTDC_PCPOLARITY_IPC;

    /* LTDC timing registers hold accumulated widths minus one. */
    hltdc->Init.HorizontalSync = LTDC_HSYNC_WIDTH - 1;
    hltdc->Init.VerticalSync = LTDC_VSYNC_HEIGHT - 1;
    hltdc->Init.AccumulatedHBP = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH - 1;
    hltdc->Init.AccumulatedVBP = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT - 1;
    hltdc->Init.AccumulatedActiveW = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + config->width - 1;
    hltdc->Init.AccumulatedActiveH = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + config->height - 1;
    hltdc->Init.TotalWidth = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + config->width + LTDC_HFP_WIDTH - 1;
    hltdc->Init.TotalHeigh = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + config->height + LTDC_VFP_HEIGHT - 1;

    LTDC_UnpackBackcolor(config->backgroundColor, &hltdc->Init.Backcolor);

    if (hltdc->State == HAL_LTDC_STATE_RESET) {
        HAL_StatusTypeDef status = HAL_LTDC_Init(hltdc);
        if (status != HAL_OK) {
            driver->errorCode = LTDC_ERROR_INIT_FAILED;
            return status;
        }
    }

    driver->displayConfig = *config;

    log_debug("LTDC: Display configured successfully");
    return HAL_OK;
}

/* Display control -----------------------------------------------------------*/

HAL_StatusTypeDef LTDC_SetBackgroundColor(LTDC_Driver_t *driver, uint32_t color)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_ColorTypeDef backcolor;
    LTDC_UnpackBackcolor(color, &backcolor);

    driver->hltdc->Instance->BCCR = ((uint32_t)backcolor.Red << 16) |
                                    ((uint32_t)backcolor.Green << 8) |
                                    (uint32_t)backcolor.Blue;
    __HAL_LTDC_RELOAD_CONFIG(driver->hltdc);

    driver->displayConfig.backgroundColor = color;
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_DisplayOn(LTDC_Driver_t *driver)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    __HAL_LTDC_ENABLE(driver->hltdc);
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_DisplayOff(LTDC_Driver_t *driver)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    __HAL_LTDC_DISABLE(driver->hltdc);
    return HAL_OK;
}

/* Error handling ------------------------------------------------------------*/

uint32_t LTDC_GetError(LTDC_Driver_t *driver)
{
    if (driver == NULL) {
        return LTDC_ERROR_INVALID_PARAM;
    }
    return driver->errorCode;
}

HAL_StatusTypeDef LTDC_ClearError(LTDC_Driver_t *driver)
{
    if (driver == NULL) {
        return HAL_ERROR;
    }
    driver->errorCode = LTDC_ERROR_NONE;
    return HAL_OK;
}

/* Shared validation ---------------------------------------------------------*/

HAL_StatusTypeDef LTDC_ValidateDriver(LTDC_Driver_t *driver)
{
    if (driver == NULL || !driver->initialized || driver->hltdc == NULL) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_ValidateLayer(uint8_t layer)
{
    return (layer < LTDC_MAX_LAYERS) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef LTDC_ValidateCoordinates(uint16_t xCoord, uint16_t yCoord)
{
    if (xCoord >= LTDC_DISPLAY_WIDTH || yCoord >= LTDC_DISPLAY_HEIGHT) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_ValidateRect(LTDC_Rect_t *rect)
{
    if (rect == NULL ||
        rect->x >= LTDC_DISPLAY_WIDTH ||
        rect->y >= LTDC_DISPLAY_HEIGHT ||
        (rect->x + rect->width) > LTDC_DISPLAY_WIDTH ||
        (rect->y + rect->height) > LTDC_DISPLAY_HEIGHT) {
        return HAL_ERROR;
    }
    return HAL_OK;
}
