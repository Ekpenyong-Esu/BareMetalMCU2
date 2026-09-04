/**
 ******************************************************************************
 * @file    lv_port_disp.h
 * @brief   LVGL display driver for the STM32F429I LCD
 * @details Sets up the LVGL display so it can draw on the board's LCD.
 *          Normally you don't call this directly — use LVGL_App_Init() instead.
 ******************************************************************************
 */

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set up the LVGL display driver for the STM32F429I LCD
 * @note  Called automatically by LVGL_App_Init(); only call directly if you
 *        need custom display setup after lv_init()
 */
void lv_port_disp_init(void);

#ifdef __cplusplus
}
#endif

#endif /* LV_PORT_DISP_H */
