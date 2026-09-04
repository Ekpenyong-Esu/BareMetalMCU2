/**
 ******************************************************************************
 * @file    lvgl_app_nav.h
 * @brief   Navigation helpers — wire buttons to screens
 * @details Buttons store a screen ID (not a pointer), so a button can
 *          target a screen that hasn't been created yet.
 ******************************************************************************
 */

#ifndef LVGL_APP_NAV_H
#define LVGL_APP_NAV_H

#include "lvgl.h"
#include "lvgl_app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Make a button switch to another screen when clicked
 * @param btn     Button to wire up
 * @param target  Screen to show when the button is pressed
 */
void LVGL_Nav_Attach(lv_obj_t *btn, LVGL_ScreenId_t target);

/**
 * @brief Create a standard back button (top-left, already wired)
 * @param parent  Parent widget to place the button in
 * @param target  Screen to go back to
 * @retval lv_obj_t* The new button object
 */
lv_obj_t *LVGL_Nav_CreateBackButton(lv_obj_t *parent, LVGL_ScreenId_t target);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_NAV_H */
