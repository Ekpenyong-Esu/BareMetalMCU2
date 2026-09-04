/*******************************************************************************
 * LVGL Application Core - lifecycle and public update API
 *******************************************************************************
 * Quick Start:
 * 1. Call LVGL_App_Init() once after HAL_Init() and SystemClock_Config()
 * 2. Call LVGL_App_Tick() every 1-5 ms in your main loop
 ******************************************************************************/

#ifndef LVGL_APP_CORE_H
#define LVGL_APP_CORE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise LVGL, the display and input ports, and build the GUI.
 * @note   Safe to call again; subsequent calls are ignored.
 */
void LVGL_App_Init(void);

/** @brief Process LVGL timers, rendering and animations. Call every 1-5 ms. */
void LVGL_App_Tick(void);

bool LVGL_App_IsInitialized(void);

/** @brief Temperature in Celsius; clamped to the gauge range. */
void LVGL_App_UpdateTemperature(int temp_celsius);

/** @brief Relative humidity in percent; clamped to the bar range. */
void LVGL_App_UpdateHumidity(int humidity_percent);

void LVGL_App_UpdateStatus(const char *status);

/** @brief Append a point to the sensor chart; clamped to the chart range. */
void LVGL_App_AddChartData(int value);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_CORE_H */
