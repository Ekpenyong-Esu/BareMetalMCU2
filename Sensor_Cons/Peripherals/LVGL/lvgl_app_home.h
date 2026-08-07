/*******************************************************************************
 * LVGL Application - Home dashboard screen
 ******************************************************************************/

#ifndef LVGL_APP_HOME_H
#define LVGL_APP_HOME_H

#ifdef __cplusplus
extern "C" {
#endif

void LVGL_Home_Create(void);

/** Values outside the gauge range are clamped, so gauge and label agree. */
void LVGL_Home_SetTemperature(int temp_celsius);
void LVGL_Home_SetHumidity(int humidity_percent);
void LVGL_Home_SetStatus(const char *status);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_HOME_H */
