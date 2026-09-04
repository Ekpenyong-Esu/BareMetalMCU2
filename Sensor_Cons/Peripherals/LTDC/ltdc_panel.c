/**
 * @file ltdc_panel.c
 * @brief One-call panel bring-up over an application-supplied framebuffer
 */

#include "ltdc_panel.h"
#include "ltdc_core.h"
#include "ltdc_layer.h"
#include "ltdc_pixel.h"
#include "log.h"
#include <string.h>

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Describe layer 0 as a full-screen layer over the caller's framebuffer
 * @param panel Panel description
 * @param layer Destination driver layer configuration
 */
static void LTDC_PanelBuildLayer0(const LTDC_PanelConfig_t *panel, LTDC_LayerConfig_t *layer) {
    layer->framebufferAddress = panel->framebufferAddress;
    layer->windowX0 = 0;
    layer->windowY0 = 0;
    layer->windowX1 = panel->display.width - 1; /* inclusive end coordinate */
    layer->windowY1 = panel->display.height - 1;
    layer->imageWidth = panel->display.width;
    layer->imageHeight = panel->display.height;
    layer->pixelFormat = panel->pixelFormat;
    layer->alpha = LTDC_MAX_ALPHA;
    layer->alpha0 = 0;
    layer->blendMode = LTDC_BLEND_PIXEL_ALPHA;
    layer->backgroundColor = LTDC_COLOR_BLACK;
    layer->enabled = false;
}

/* Public functions ----------------------------------------------------------*/

LTDC_DisplayConfig_t LTDC_PanelDefaultsILI9341(void) {
    LTDC_DisplayConfig_t config = {0};

    config.width = LTDC_ILI9341_WIDTH;
    config.height = LTDC_ILI9341_HEIGHT;
    config.hsyncWidth = LTDC_ILI9341_HSYNC_WIDTH;
    config.hbp = LTDC_ILI9341_HBP_WIDTH;
    config.hfp = LTDC_ILI9341_HFP_WIDTH;
    config.vsyncHeight = LTDC_ILI9341_VSYNC_HEIGHT;
    config.vbp = LTDC_ILI9341_VBP_HEIGHT;
    config.vfp = LTDC_ILI9341_VFP_HEIGHT;
    config.backgroundColor = LTDC_COLOR_BLACK;

    /* Polarities must match the ILI9341 RGB interface. */
    config.hsyncActiveLow = true;
    config.vsyncActiveLow = true;
    config.dataEnableActiveLow = true;
    config.pixelClockInverted = false;

    return config;
}

HAL_StatusTypeDef LTDC_PanelInit(LTDC_Driver_t *driver, const LTDC_PanelConfig_t *panel) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }
    if (panel == NULL || panel->framebufferAddress == 0U) {
        log_error("LTDC: Panel config needs a framebuffer");
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    if (LTDC_ConfigureDisplay(driver, &panel->display) != HAL_OK) {
        log_error("LTDC: LTDC init failed");
        return HAL_ERROR;
    }

    /* Whatever the framebuffer held before must not flash on screen. */
    const uint32_t frameBytes =
        (uint32_t)panel->display.width * panel->display.height * LTDC_PixelSize(panel->pixelFormat);
    memset((void *)(uintptr_t)panel->framebufferAddress, 0, frameBytes);

    LTDC_LayerConfig_t layer;
    LTDC_PanelBuildLayer0(panel, &layer);

    if (LTDC_ConfigureLayer(driver, 0, &layer) != HAL_OK) {
        log_error("LTDC: Layer config error");
        return HAL_ERROR;
    }

    log_debug("LTDC: Panel initialized successfully");
    return HAL_OK;
}
