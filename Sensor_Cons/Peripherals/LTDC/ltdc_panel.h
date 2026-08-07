/**
 * @file ltdc_panel.h
 * @brief Board bring-up of the on-board ILI9341 RGB panel
 */

#ifndef LTDC_PANEL_H
#define LTDC_PANEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

/**
 * @brief Bring up the STM32F429I-DISC1 panel on layer 0 with an SDRAM framebuffer
 * @details Clocks and GPIOs come from HAL_LTDC_MspInit(); this only programs the
 *          controller and clears the framebuffer.
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_HW_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_PANEL_H */
