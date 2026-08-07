/*******************************************************************************
 * LVGL Application - Sensor monitoring screen
 ******************************************************************************/

#ifndef LVGL_APP_SENSORS_H
#define LVGL_APP_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

void LVGL_Sensors_Create(void);

/** Values outside the chart range are clamped. */
void LVGL_Sensors_AddChartData(int value);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_SENSORS_H */
