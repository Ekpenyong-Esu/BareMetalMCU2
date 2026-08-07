/*******************************************************************************
 * LVGL Application - Settings screen
 *******************************************************************************
 * Two sliders, two toggle switches, a link to system info and a back button.
 ******************************************************************************/

#include "lvgl_app_settings.h"
#include "lvgl_app_nav.h"
#include "lvgl_app_screens.h"
#include "lvgl_app_types.h"
#include "lvgl.h"

#define SETTINGS_ROW_LEFT     20
#define SETTINGS_SLIDER_W     180
#define SETTINGS_SLIDER_H     10

static lv_obj_t *LVGL_Settings_CreateSlider(lv_obj_t *parent, const char *text,
                                            int32_t y, int32_t value, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, SETTINGS_ROW_LEFT, y);

    lv_obj_t *slider = lv_slider_create(parent);
    lv_obj_set_size(slider, SETTINGS_SLIDER_W, SETTINGS_SLIDER_H);
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, SETTINGS_ROW_LEFT, y + 25);
    lv_slider_set_value(slider, value, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(color), LV_PART_INDICATOR);

    return slider;
}

static lv_obj_t *LVGL_Settings_CreateSwitch(lv_obj_t *parent, const char *text,
                                            int32_t y, bool checked)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, SETTINGS_ROW_LEFT, y);

    lv_obj_t *sw = lv_switch_create(parent);
    lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -SETTINGS_ROW_LEFT, y - 5);
    if (checked) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_set_style_bg_color(sw, lv_color_hex(UI_COLOR_ACCENT), LV_PART_INDICATOR);

    return sw;
}

void LVGL_Settings_Create(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BG), 0);
    LVGL_Screens_Register(LVGL_SCREEN_SETTINGS, screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    (void)LVGL_Settings_CreateSlider(screen, "Brightness", 50, 70, UI_COLOR_WARN);
    (void)LVGL_Settings_CreateSlider(screen, "Volume", 110, 50, UI_COLOR_TEAL);

    (void)LVGL_Settings_CreateSwitch(screen, "WiFi", 170, false);
    (void)LVGL_Settings_CreateSwitch(screen, "Bluetooth", 210, true);

    lv_obj_t *btn_info = lv_btn_create(screen);
    lv_obj_set_size(btn_info, 200, 40);
    lv_obj_align(btn_info, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn_info, lv_color_hex(UI_COLOR_INFO), 0);
    LVGL_Nav_Attach(btn_info, LVGL_SCREEN_INFO);

    lv_obj_t *lbl_info = lv_label_create(btn_info);
    lv_label_set_text(lbl_info, LV_SYMBOL_CALL " System Info");
    lv_obj_center(lbl_info);

    (void)LVGL_Nav_CreateBackButton(screen, LVGL_SCREEN_HOME);
}
