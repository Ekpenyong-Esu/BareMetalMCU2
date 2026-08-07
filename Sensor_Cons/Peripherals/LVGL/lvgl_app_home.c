/*******************************************************************************
 * LVGL Application - Home dashboard screen
 *******************************************************************************
 * Title, status card, temperature arc gauge, humidity bar and two nav buttons.
 ******************************************************************************/

#include "lvgl_app_home.h"
#include "lvgl_app_nav.h"
#include "lvgl_app_screens.h"
#include "lvgl_app_types.h"
#include "lvgl.h"

#define HOME_VALUE_TEXT_LEN   16

static lv_obj_t *s_status_label;
static lv_obj_t *s_temp_arc;
static lv_obj_t *s_temp_label;
static lv_obj_t *s_humidity_bar;
static lv_obj_t *s_humidity_label;

static int LVGL_Home_Clamp(int value, int min, int max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static void LVGL_Home_CreateStatusCard(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 150, 40);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_color(card, lv_color_hex(UI_COLOR_CARD), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(UI_COLOR_BORDER), 0);
    lv_obj_set_style_border_width(card, 2, 0);

    s_status_label = lv_label_create(card);
    lv_label_set_text(s_status_label, "System Ready");
    lv_obj_set_style_text_color(s_status_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_status_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(s_status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(s_status_label);
}

static void LVGL_Home_CreateTemperatureGauge(lv_obj_t *parent)
{
    s_temp_arc = lv_arc_create(parent);
    lv_obj_set_size(s_temp_arc, 80, 80);
    lv_obj_align(s_temp_arc, LV_ALIGN_TOP_LEFT, 20, 140);
    lv_arc_set_rotation(s_temp_arc, 135);
    lv_arc_set_bg_angles(s_temp_arc, 0, 270);
    lv_arc_set_range(s_temp_arc, UI_TEMP_MIN_C, UI_TEMP_MAX_C);
    lv_arc_set_value(s_temp_arc, 25);
    lv_obj_set_style_arc_color(s_temp_arc, lv_color_hex(UI_COLOR_TEMP), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_temp_arc, 8, LV_PART_INDICATOR);

    s_temp_label = lv_label_create(parent);
    lv_label_set_text(s_temp_label, "25\u00B0C\nTemp");
    lv_obj_set_style_text_color(s_temp_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_temp_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(s_temp_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(s_temp_label, s_temp_arc, LV_ALIGN_CENTER, 0, 0);
}

static void LVGL_Home_CreateHumidityGauge(lv_obj_t *parent)
{
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, 80, 80);
    lv_obj_align(container, LV_ALIGN_TOP_RIGHT, -20, 130);
    lv_obj_set_style_bg_color(container, lv_color_hex(UI_COLOR_CARD), 0);
    lv_obj_set_style_border_width(container, 0, 0);

    s_humidity_bar = lv_bar_create(container);
    lv_obj_set_size(s_humidity_bar, 15, 60);
    lv_obj_center(s_humidity_bar);
    lv_bar_set_range(s_humidity_bar, UI_HUMIDITY_MIN_PCT, UI_HUMIDITY_MAX_PCT);
    lv_bar_set_value(s_humidity_bar, 60, LV_ANIM_ON);
    lv_obj_set_style_bg_color(s_humidity_bar, lv_color_hex(UI_COLOR_TEAL), LV_PART_INDICATOR);

    s_humidity_label = lv_label_create(container);
    lv_label_set_text(s_humidity_label, "60%\nHumid");
    lv_obj_set_style_text_color(s_humidity_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(s_humidity_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_humidity_label, LV_ALIGN_BOTTOM_MID, 0, -5);
}

static void LVGL_Home_CreateNavButtons(lv_obj_t *parent)
{
    lv_obj_t *btn_sensors = lv_btn_create(parent);
    lv_obj_set_size(btn_sensors, 100, 40);
    lv_obj_align(btn_sensors, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(btn_sensors, lv_color_hex(UI_COLOR_ACCENT), 0);
    LVGL_Nav_Attach(btn_sensors, LVGL_SCREEN_SENSORS);

    lv_obj_t *lbl_sensors = lv_label_create(btn_sensors);
    lv_label_set_text(lbl_sensors, LV_SYMBOL_LIST " Sensors");
    lv_obj_center(lbl_sensors);

    lv_obj_t *btn_settings = lv_btn_create(parent);
    lv_obj_set_size(btn_settings, 100, 40);
    lv_obj_align(btn_settings, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(btn_settings, lv_color_hex(UI_COLOR_WARN), 0);
    LVGL_Nav_Attach(btn_settings, LVGL_SCREEN_SETTINGS);

    lv_obj_t *lbl_settings = lv_label_create(btn_settings);
    lv_label_set_text(lbl_settings, LV_SYMBOL_SETTINGS " Config");
    lv_obj_center(lbl_settings);
}

void LVGL_Home_Create(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BG), 0);
    LVGL_Screens_Register(LVGL_SCREEN_HOME, screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "STM32F429 Dashboard");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    LVGL_Home_CreateStatusCard(screen);
    LVGL_Home_CreateTemperatureGauge(screen);
    LVGL_Home_CreateHumidityGauge(screen);
    LVGL_Home_CreateNavButtons(screen);
}

void LVGL_Home_SetTemperature(int temp_celsius)
{
    char text[HOME_VALUE_TEXT_LEN];

    temp_celsius = LVGL_Home_Clamp(temp_celsius, UI_TEMP_MIN_C, UI_TEMP_MAX_C);

    if (s_temp_arc != NULL) {
        lv_arc_set_value(s_temp_arc, temp_celsius);
    }

    if (s_temp_label != NULL) {
        lv_snprintf(text, sizeof(text), "%d\u00B0C\nTemp", temp_celsius);
        lv_label_set_text(s_temp_label, text);
    }
}

void LVGL_Home_SetHumidity(int humidity_percent)
{
    char text[HOME_VALUE_TEXT_LEN];

    humidity_percent = LVGL_Home_Clamp(humidity_percent,
                                       UI_HUMIDITY_MIN_PCT, UI_HUMIDITY_MAX_PCT);

    if (s_humidity_bar != NULL) {
        lv_bar_set_value(s_humidity_bar, humidity_percent, LV_ANIM_ON);
    }

    if (s_humidity_label != NULL) {
        lv_snprintf(text, sizeof(text), "%d%%\nHumid", humidity_percent);
        lv_label_set_text(s_humidity_label, text);
    }
}

void LVGL_Home_SetStatus(const char *status)
{
    if (s_status_label != NULL && status != NULL) {
        lv_label_set_text(s_status_label, status);
    }
}
