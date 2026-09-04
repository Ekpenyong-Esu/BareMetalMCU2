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

#define SETTINGS_ROW_LEFT 20
#define SETTINGS_SLIDER_W 180
#define SETTINGS_SLIDER_H 10

/* Layout constants (pixels unless stated) */
#define GUI_SETTINGS_TITLE_Y 10            /* Title offset from the top edge */
#define GUI_SETTINGS_SLIDER_LABEL_GAP 25   /* Slider offset below its label */
#define GUI_SETTINGS_SWITCH_LABEL_LIFT 5   /* Switch raised above its label baseline */
#define GUI_SETTINGS_BRIGHTNESS_ROW_Y 50   /* Brightness label row */
#define GUI_SETTINGS_VOLUME_ROW_Y 110      /* Volume label row */
#define GUI_SETTINGS_WIFI_ROW_Y 170        /* WiFi switch row */
#define GUI_SETTINGS_BLUETOOTH_ROW_Y 210   /* Bluetooth switch row */
#define GUI_SETTINGS_BRIGHTNESS_INITIAL 70 /* Slider value before the first update (percent) */
#define GUI_SETTINGS_VOLUME_INITIAL 50     /* Slider value before the first update (percent) */
#define GUI_SETTINGS_INFO_BTN_W 200        /* System-info button width */
#define GUI_SETTINGS_INFO_BTN_H 40         /* System-info button height */
#define GUI_SETTINGS_INFO_BTN_BOTTOM (-20) /* System-info button offset from the bottom edge */

static lv_obj_t *LVGL_Settings_CreateSlider(lv_obj_t *parent, const char *text, int32_t rowY,
                                            int32_t value, uint32_t color) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, SETTINGS_ROW_LEFT, rowY);

    lv_obj_t *slider = lv_slider_create(parent);
    lv_obj_set_size(slider, SETTINGS_SLIDER_W, SETTINGS_SLIDER_H);
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, SETTINGS_ROW_LEFT,
                 rowY + GUI_SETTINGS_SLIDER_LABEL_GAP);
    lv_slider_set_value(slider, value, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(color), LV_PART_INDICATOR);

    return slider;
}

static lv_obj_t *LVGL_Settings_CreateSwitch(lv_obj_t *parent, const char *text, int32_t rowY,
                                            bool checked) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, SETTINGS_ROW_LEFT, rowY);

    lv_obj_t *toggle = lv_switch_create(parent);
    lv_obj_align(toggle, LV_ALIGN_TOP_RIGHT, -SETTINGS_ROW_LEFT,
                 rowY - GUI_SETTINGS_SWITCH_LABEL_LIFT);
    if (checked) {
        lv_obj_add_state(toggle, LV_STATE_CHECKED);
    }
    lv_obj_set_style_bg_color(toggle, lv_color_hex(UI_COLOR_ACCENT), LV_PART_INDICATOR);

    return toggle;
}

void LVGL_Settings_Create(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BG), 0);
    LVGL_Screens_Register(LVGL_SCREEN_SETTINGS, screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, GUI_SETTINGS_TITLE_Y);

    (void)LVGL_Settings_CreateSlider(screen, "Brightness", GUI_SETTINGS_BRIGHTNESS_ROW_Y,
                                     GUI_SETTINGS_BRIGHTNESS_INITIAL, UI_COLOR_WARN);
    (void)LVGL_Settings_CreateSlider(screen, "Volume", GUI_SETTINGS_VOLUME_ROW_Y,
                                     GUI_SETTINGS_VOLUME_INITIAL, UI_COLOR_TEAL);

    (void)LVGL_Settings_CreateSwitch(screen, "WiFi", GUI_SETTINGS_WIFI_ROW_Y, false);
    (void)LVGL_Settings_CreateSwitch(screen, "Bluetooth", GUI_SETTINGS_BLUETOOTH_ROW_Y, true);

    lv_obj_t *btn_info = lv_btn_create(screen);
    lv_obj_set_size(btn_info, GUI_SETTINGS_INFO_BTN_W, GUI_SETTINGS_INFO_BTN_H);
    lv_obj_align(btn_info, LV_ALIGN_BOTTOM_MID, 0, GUI_SETTINGS_INFO_BTN_BOTTOM);
    lv_obj_set_style_bg_color(btn_info, lv_color_hex(UI_COLOR_INFO), 0);
    LVGL_Nav_Attach(btn_info, LVGL_SCREEN_INFO);

    lv_obj_t *lbl_info = lv_label_create(btn_info);
    lv_label_set_text(lbl_info, LV_SYMBOL_CALL " System Info");
    lv_obj_center(lbl_info);

    (void)LVGL_Nav_CreateBackButton(screen, LVGL_SCREEN_HOME);
}
