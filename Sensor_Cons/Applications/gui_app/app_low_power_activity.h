/**
 ******************************************************************************
 * @file    app_low_power_activity.h
 * @brief   User activity tracking and the inactivity policy it drives
 ******************************************************************************
 */

#ifndef APP_LOW_POWER_ACTIVITY_H
#define APP_LOW_POWER_ACTIVITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_low_power_types.h"
#include "pwr.h"

/**
 * @brief Restart the activity timer and clear the touch debounce window.
 */
void APP_ActivityReset(void);

/**
 * @brief Reset the activity timer. Call on any non-touch user interaction.
 */
void APP_UpdateActivity(void);

/**
 * @brief Reset the activity timer from a touch event and undo dimming/blanking.
 * @note  Debounced by APP_TOUCH_ACTIVITY_DEBOUNCE_MS.
 */
void APP_TouchActivity(void);

/**
 * @brief Advance the inactivity ladder and report whether to sleep.
 * @note  Not a pure predicate: it dims and then blanks the display as the
 *        deadlines pass, returning false until the display is off.
 */
bool APP_ShouldEnterLowPower(void);

/**
 * @brief Enter Stop or Standby depending on how long the system has been idle.
 */
PWR_StatusTypeDef APP_EnterLowPowerMode(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_LOW_POWER_ACTIVITY_H */
