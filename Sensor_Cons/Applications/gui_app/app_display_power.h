/**
 ******************************************************************************
 * @file    app_display_power.h
 * @brief   Backlight, panel and LTDC power states with fade transitions
 * @note    Internal to the low power manager; not part of app_low_power.h.
 ******************************************************************************
 */

#ifndef APP_DISPLAY_POWER_H
#define APP_DISPLAY_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_low_power_types.h"

/**
 * @brief Configure the backlight pin and force the display on.
 */
void APP_Display_Init(void);

/**
 * @brief Start the fade to black that ends in panel sleep and LTDC off.
 * @note  Returns immediately; the state only changes when the LVGL animation
 *        completes, so poll APP_Display_IsFading() before cutting power.
 */
void APP_Display_PowerOff(void);

/**
 * @brief Wake the panel, re-enable LTDC and fade back in.
 */
void APP_Display_PowerOn(void);

/**
 * @brief Fade the backlight out (dim) or back in, without touching the panel.
 */
void APP_Display_Dim(bool dim);

bool APP_Display_IsOn(void);
bool APP_Display_IsDimmed(void);

/**
 * @brief True while a fade overlay still exists and needs LVGL to run.
 */
bool APP_Display_IsFading(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_DISPLAY_POWER_H */
