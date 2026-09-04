/**
 ******************************************************************************
 * @file    lvgl_app_home.h
 * @brief   Home dashboard screen — shows temperature, humidity and status
 * @details The first screen the user sees. Has gauges for temperature and
 *          humidity, plus a status label at the bottom.
 *
 * How it works (in simple words):
 * - LVGL_Home_Create builds the screen with gauges and labels.
 * - The other functions update the gauges — values outside the range are
 *   clamped so the gauge and label always agree.
 ******************************************************************************
 */

#ifndef LVGL_APP_HOME_H
#define LVGL_APP_HOME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build the home screen (gauges, labels, layout)
 */
void LVGL_Home_Create(void);

/**
 * @brief Update the temperature gauge and label
 * @param temp_celsius  Temperature in degrees C (clamped to gauge range)
 */
void LVGL_Home_SetTemperature(int temp_celsius);

/**
 * @brief Update the humidity gauge and label
 * @param humidity_percent  Humidity in % (clamped to gauge range)
 */
void LVGL_Home_SetHumidity(int humidity_percent);

/**
 * @brief Update the status text at the bottom of the home screen
 * @param status  Text to show (e.g. "Ready", "Sensor error")
 */
void LVGL_Home_SetStatus(const char *status);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_HOME_H */
