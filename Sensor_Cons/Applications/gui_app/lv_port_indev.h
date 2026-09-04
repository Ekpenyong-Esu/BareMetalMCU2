/**
 ******************************************************************************
 * @file    lv_port_indev.h
 * @brief   LVGL touchscreen driver for the STM32F429I
 * @details Sets up the LVGL input device so touch events reach the GUI.
 *          Normally you don't call this directly — use LVGL_App_Init() instead.
 ******************************************************************************
 */

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set up the LVGL touchscreen driver
 * @note  Called automatically by LVGL_App_Init(); only call directly if you
 *        need custom input setup after lv_init()
 */
void lv_port_indev_init(void);

#ifdef __cplusplus
}
#endif

#endif /* LV_PORT_INDEV_H */
