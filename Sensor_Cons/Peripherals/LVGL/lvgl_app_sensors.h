/**
  ******************************************************************************
  * @file    lvgl_app_sensors.h
  * @brief   Sensor monitoring screen — shows a chart of sensor readings
  * @details Has a chart that plots sensor values over time. New data is
  *          added with LVGL_Sensors_AddChartData().
  ******************************************************************************
  */

#ifndef LVGL_APP_SENSORS_H
#define LVGL_APP_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build the sensor screen (chart, labels, layout)
 */
void LVGL_Sensors_Create(void);

/**
 * @brief Add a new data point to the sensor chart
 * @param value  Sensor value to plot (clamped to chart range)
 */
void LVGL_Sensors_AddChartData(int value);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_SENSORS_H */
