/**
 * @file ltdc_panel.h
 * @brief One-call panel bring-up over an application-supplied framebuffer
 * @details The application decides where the framebuffer lives and which
 *          panel is attached; the ILI9341 defaults below describe the
 *          STM32F429I-DISC1 display but are only applied when passed in.
 */

#ifndef LTDC_PANEL_H
#define LTDC_PANEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

/* ILI9341 RGB interface facts (true widths, not accumulated register values) */
#define LTDC_ILI9341_WIDTH 240      /*!< Active width in pixels */
#define LTDC_ILI9341_HEIGHT 320     /*!< Active height in lines */
#define LTDC_ILI9341_HSYNC_WIDTH 10 /*!< Horizontal sync width */
#define LTDC_ILI9341_HBP_WIDTH 20   /*!< Horizontal back porch */
#define LTDC_ILI9341_HFP_WIDTH 10   /*!< Horizontal front porch */
#define LTDC_ILI9341_VSYNC_HEIGHT 2 /*!< Vertical sync height */
#define LTDC_ILI9341_VBP_HEIGHT 2   /*!< Vertical back porch */
#define LTDC_ILI9341_VFP_HEIGHT 4   /*!< Vertical front porch */

/**
 * @brief Everything LTDC_PanelInit() needs that only the application knows
 */
typedef struct {
    uint32_t framebufferAddress;    /*!< Layer 0 framebuffer; must not be 0 */
    LTDC_PixelFormat_t pixelFormat; /*!< Framebuffer pixel format; RGB888 is rejected */
    LTDC_DisplayConfig_t display;   /*!< Panel geometry, timings and polarities */
} LTDC_PanelConfig_t;

/**
 * @brief Geometry, timings and polarities of the on-board ILI9341 panel
 * @return LTDC_DisplayConfig_t Copy it and override only what differs
 */
LTDC_DisplayConfig_t LTDC_PanelDefaultsILI9341(void);

/**
 * @brief Program the panel timings, clear the framebuffer and put it on layer 0
 * @details Clocks and GPIOs come from HAL_LTDC_MspInit(); this only programs
 *          the controller. The driver must already be bound with
 *          LTDC_Driver_Init().
 * @param driver Driver record
 * @param panel Framebuffer and display description
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_PanelInit(LTDC_Driver_t *driver, const LTDC_PanelConfig_t *panel);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_PANEL_H */
