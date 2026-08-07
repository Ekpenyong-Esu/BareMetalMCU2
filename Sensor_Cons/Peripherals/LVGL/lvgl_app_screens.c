/*******************************************************************************
 * LVGL Application - screen registry
 ******************************************************************************/

#include "lvgl_app_screens.h"

static lv_obj_t *s_screens[LVGL_SCREEN_COUNT];

void LVGL_Screens_Register(LVGL_ScreenId_t id, lv_obj_t *screen)
{
    if (id < LVGL_SCREEN_COUNT) {
        s_screens[id] = screen;
    }
}

lv_obj_t *LVGL_Screens_Get(LVGL_ScreenId_t id)
{
    return (id < LVGL_SCREEN_COUNT) ? s_screens[id] : NULL;
}

void LVGL_Screens_Reset(void)
{
    for (int i = 0; i < LVGL_SCREEN_COUNT; i++) {
        s_screens[i] = NULL;
    }
}
