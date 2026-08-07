/*******************************************************************************
 * LVGL Application - screen registry
 *******************************************************************************
 * Owns the mapping from LVGL_ScreenId_t to the live screen object, so screens
 * can reference each other without depending on creation order.
 ******************************************************************************/

#ifndef LVGL_APP_SCREENS_H
#define LVGL_APP_SCREENS_H

#include "lvgl.h"
#include "lvgl_app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void LVGL_Screens_Register(LVGL_ScreenId_t id, lv_obj_t *screen);

/** @return the registered screen, or NULL if that id has not been created. */
lv_obj_t *LVGL_Screens_Get(LVGL_ScreenId_t id);

void LVGL_Screens_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_SCREENS_H */
