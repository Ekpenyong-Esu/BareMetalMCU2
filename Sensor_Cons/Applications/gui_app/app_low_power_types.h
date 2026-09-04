/**
 ******************************************************************************
 * @file    app_low_power_types.h
 * @brief   Inactivity policy constants for the application low power manager
 ******************************************************************************
 */

#ifndef APP_LOW_POWER_TYPES_H
#define APP_LOW_POWER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** Inactivity ladder, in milliseconds. Each step must exceed the previous one. */
#define APP_DISPLAY_DIM_TIMEOUT_MS 5000U
#define APP_DISPLAY_OFF_TIMEOUT_MS 10000U
#define APP_LOW_POWER_TIMEOUT_MS 15000U

/** Below this inactivity the manager picks Stop mode, above it Standby. */
#define APP_LOW_POWER_SHORT_MS 120000U

/** Some hardware reports one touch twice: once via EXTI, once via LVGL sampling. */
#define APP_TOUCH_ACTIVITY_DEBOUNCE_MS 80U

#ifdef __cplusplus
}
#endif

#endif /* APP_LOW_POWER_TYPES_H */
