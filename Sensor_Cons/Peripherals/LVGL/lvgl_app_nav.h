/*******************************************************************************
 * LVGL Application - navigation
 *******************************************************************************
 * Buttons carry a screen *id*, not a screen pointer, so a button may target a
 * screen that has not been created yet.
 ******************************************************************************/

#ifndef LVGL_APP_NAV_H
#define LVGL_APP_NAV_H

#include "lvgl.h"
#include "lvgl_app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Make a click on @p btn load the screen registered under @p target. */
void LVGL_Nav_Attach(lv_obj_t *btn, LVGL_ScreenId_t target);

/** Standard top-left back button, already wired to @p target. */
lv_obj_t *LVGL_Nav_CreateBackButton(lv_obj_t *parent, LVGL_ScreenId_t target);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_NAV_H */
