/*******************************************************************************
 * LVGL Application - screen registry
 ******************************************************************************/

#include "lvgl_app_screens.h"

static lv_obj_t *s_screens[LVGL_SCREEN_COUNT];

void LVGL_Screens_Register(LVGL_ScreenId_t screenId, lv_obj_t *screen) {
    if (screenId < LVGL_SCREEN_COUNT) {
        s_screens[screenId] = screen;
    }
}

lv_obj_t *LVGL_Screens_Get(LVGL_ScreenId_t screenId) {
    return (screenId < LVGL_SCREEN_COUNT) ? s_screens[screenId] : NULL;
}

void LVGL_Screens_Reset(void) {
    for (int i = 0; i < LVGL_SCREEN_COUNT; i++) {
        s_screens[i] = NULL;
    }
}
