/*******************************************************************************
 * LVGL Application - navigation
 ******************************************************************************/

#include <stdint.h>

#include "lvgl_app_nav.h"
#include "lvgl_app_screens.h"

#define NAV_ANIM_TIME_MS 300
#define NAV_BUTTON_WIDTH 60
#define NAV_BUTTON_HEIGHT 35
#define NAV_BUTTON_MARGIN 5

static void LVGL_Nav_EventHandler(lv_event_t *event) {
    uintptr_t screenId = 0;
    lv_obj_t *target = NULL;

    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    screenId = (uintptr_t)lv_event_get_user_data(event);
    if (screenId >= (uintptr_t)LVGL_SCREEN_COUNT) {
        return;
    }

    target = LVGL_Screens_Get((LVGL_ScreenId_t)screenId);
    if (target == NULL) {
        return;
    }

    lv_screen_load_anim(target, LV_SCR_LOAD_ANIM_FADE_ON, NAV_ANIM_TIME_MS, 0, false);
}

void LVGL_Nav_Attach(lv_obj_t *btn, LVGL_ScreenId_t target) {
    if (btn == NULL || target >= LVGL_SCREEN_COUNT) {
        return;
    }

    lv_obj_add_event_cb(btn, LVGL_Nav_EventHandler, LV_EVENT_CLICKED, (void *)(uintptr_t)target);
}

lv_obj_t *LVGL_Nav_CreateBackButton(lv_obj_t *parent, LVGL_ScreenId_t target) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, NAV_BUTTON_MARGIN, NAV_BUTTON_MARGIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_NEUTRAL), 0);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl);

    LVGL_Nav_Attach(btn, target);

    return btn;
}
