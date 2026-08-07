/**
 * @file ltdc_panel.c
 * @brief Board bring-up of the on-board ILI9341 RGB panel
 */

#include "ltdc_panel.h"
#include "ltdc_core.h"
#include "fmc.h"
#include "log.h"
#include <string.h>

#define LTDC_PANEL_FB_ADDRESS       ((uint32_t)SDRAM_DEVICE_ADDR)    /*!< Layer 0 framebuffer in external SDRAM */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Program the controller timings for the on-board panel
 * @param hltdc HAL handle to configure
 */
static void LTDC_PanelSetTimings(LTDC_HandleTypeDef *hltdc)
{
    hltdc->Instance = LTDC;

    /* Polarities must match the ILI9341 RGB interface. */
    hltdc->Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hltdc->Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hltdc->Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hltdc->Init.PCPolarity = LTDC_PCPOLARITY_IPC;

    /* ST BSP timings; registers hold accumulated widths minus one. */
    hltdc->Init.HorizontalSync = LTDC_HSYNC_WIDTH - 1;
    hltdc->Init.VerticalSync = LTDC_VSYNC_HEIGHT - 1;
    hltdc->Init.AccumulatedHBP = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH - 1;
    hltdc->Init.AccumulatedVBP = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT - 1;
    hltdc->Init.AccumulatedActiveW = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + LTDC_DISPLAY_WIDTH - 1;
    hltdc->Init.AccumulatedActiveH = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + LTDC_DISPLAY_HEIGHT - 1;
    hltdc->Init.TotalWidth = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + LTDC_DISPLAY_WIDTH + LTDC_HFP_WIDTH - 1;
    hltdc->Init.TotalHeigh = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + LTDC_DISPLAY_HEIGHT + LTDC_VFP_HEIGHT - 1;

    hltdc->Init.Backcolor.Blue = 0;
    hltdc->Init.Backcolor.Green = 0;
    hltdc->Init.Backcolor.Red = 0;
}

/**
 * @brief Describe layer 0 as a full-screen RGB565 layer over the SDRAM framebuffer
 * @param layerCfg Destination HAL structure
 */
static void LTDC_PanelSetLayer0(LTDC_LayerCfgTypeDef *layerCfg)
{
    layerCfg->WindowX0 = 0;
    layerCfg->WindowX1 = LTDC_DISPLAY_WIDTH;    /* ST BSP bring-up values, kept as-is */
    layerCfg->WindowY0 = 0;
    layerCfg->WindowY1 = LTDC_DISPLAY_HEIGHT;
    layerCfg->PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layerCfg->Alpha = 255;
    layerCfg->Alpha0 = 0;
    layerCfg->BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layerCfg->BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    layerCfg->FBStartAdress = LTDC_PANEL_FB_ADDRESS;
    layerCfg->ImageWidth = LTDC_DISPLAY_WIDTH;
    layerCfg->ImageHeight = LTDC_DISPLAY_HEIGHT;
    layerCfg->Backcolor.Blue = 0;
    layerCfg->Backcolor.Green = 0;
    layerCfg->Backcolor.Red = 0;
}

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef LTDC_HW_Init(void)
{
    LTDC_HandleTypeDef *hltdc = LTDC_GetHandle();

    LTDC_PanelSetTimings(hltdc);

    if (HAL_LTDC_Init(hltdc) != HAL_OK) {
        log_error("LTDC: LTDC init failed");
        return HAL_ERROR;
    }

    memset((void *)(uintptr_t)LTDC_PANEL_FB_ADDRESS, 0, LTDC_FB_SIZE_RGB565);

    LTDC_LayerCfgTypeDef layerCfg = {0};
    LTDC_PanelSetLayer0(&layerCfg);

    if (HAL_LTDC_ConfigLayer(hltdc, &layerCfg, 0) != HAL_OK) {
        log_error("LTDC: Layer config error");
        return HAL_ERROR;
    }

    log_debug("LTDC: Panel initialized successfully");
    return HAL_OK;
}
