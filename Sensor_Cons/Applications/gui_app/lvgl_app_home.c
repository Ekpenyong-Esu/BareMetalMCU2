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

#define HOME_VALUE_TEXT_LEN 16

/* Layout constants (pixels unless stated) for the 240x320 portrait display */
#define GUI_HOME_TITLE_Y 10              /* Title offset from the top edge */
#define GUI_HOME_STATUS_CARD_W 150       /* Status card width */
#define GUI_HOME_STATUS_CARD_H 40        /* Status card height */
#define GUI_HOME_STATUS_CARD_Y 45        /* Status card offset from the top edge */
#define GUI_HOME_GAUGE_SIZE 80           /* Temperature arc / humidity container size */
#define GUI_HOME_GAUGE_MARGIN_X 20       /* Gauge inset from the left / right edge */
#define GUI_HOME_TEMP_ARC_Y 140          /* Temperature arc offset from the top edge */
#define GUI_HOME_HUMIDITY_BOX_Y 130      /* Humidity container offset from the top edge */
#define GUI_HOME_ARC_ROTATION_DEG 135    /* Arc start angle so the gap faces down */
#define GUI_HOME_ARC_SWEEP_DEG 270       /* Arc background sweep */
#define GUI_HOME_ARC_WIDTH 8             /* Arc indicator thickness */
#define GUI_HOME_TEMP_INITIAL_C 25       /* Value shown before the first update */
#define GUI_HOME_HUMIDITY_BAR_W 15       /* Humidity bar width */
#define GUI_HOME_HUMIDITY_BAR_H 60       /* Humidity bar height */
#define GUI_HOME_HUMIDITY_INITIAL_PCT 60 /* Value shown before the first update */
#define GUI_HOME_HUMIDITY_LABEL_Y (-5)   /* Humidity label offset from the container bottom */
#define GUI_HOME_NAV_BTN_W 100           /* Navigation button width */
#define GUI_HOME_NAV_BTN_H 40            /* Navigation button height */
#define GUI_HOME_NAV_BTN_MARGIN 10       /* Navigation button inset from the screen corners */

static lv_obj_t *s_status_label;
static lv_obj_t *s_temp_arc;
static lv_obj_t *s_temp_label;
static lv_obj_t *s_humidity_bar;
static lv_obj_t *s_humidity_label;

static int LVGL_Home_Clamp(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static void LVGL_Home_CreateStatusCard(lv_obj_t *parent) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, GUI_HOME_STATUS_CARD_W, GUI_HOME_STATUS_CARD_H);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, GUI_HOME_STATUS_CARD_Y);
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

static void LVGL_Home_CreateTemperatureGauge(lv_obj_t *parent) {
    s_temp_arc = lv_arc_create(parent);
    lv_obj_set_size(s_temp_arc, GUI_HOME_GAUGE_SIZE, GUI_HOME_GAUGE_SIZE);
    lv_obj_align(s_temp_arc, LV_ALIGN_TOP_LEFT, GUI_HOME_GAUGE_MARGIN_X, GUI_HOME_TEMP_ARC_Y);
    lv_arc_set_rotation(s_temp_arc, GUI_HOME_ARC_ROTATION_DEG);
    lv_arc_set_bg_angles(s_temp_arc, 0, GUI_HOME_ARC_SWEEP_DEG);
    lv_arc_set_range(s_temp_arc, UI_TEMP_MIN_C, UI_TEMP_MAX_C);
    lv_arc_set_value(s_temp_arc, GUI_HOME_TEMP_INITIAL_C);
    lv_obj_set_style_arc_color(s_temp_arc, lv_color_hex(UI_COLOR_TEMP), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_temp_arc, GUI_HOME_ARC_WIDTH, LV_PART_INDICATOR);

    s_temp_label = lv_label_create(parent);
    lv_label_set_text(s_temp_label, "25\u00B0C\nTemp");
    lv_obj_set_style_text_color(s_temp_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_temp_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(s_temp_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(s_temp_label, s_temp_arc, LV_ALIGN_CENTER, 0, 0);
}

static void LVGL_Home_CreateHumidityGauge(lv_obj_t *parent) {
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, GUI_HOME_GAUGE_SIZE, GUI_HOME_GAUGE_SIZE);
    lv_obj_align(container, LV_ALIGN_TOP_RIGHT, -GUI_HOME_GAUGE_MARGIN_X, GUI_HOME_HUMIDITY_BOX_Y);
    lv_obj_set_style_bg_color(container, lv_color_hex(UI_COLOR_CARD), 0);
    lv_obj_set_style_border_width(container, 0, 0);

    s_humidity_bar = lv_bar_create(container);
    lv_obj_set_size(s_humidity_bar, GUI_HOME_HUMIDITY_BAR_W, GUI_HOME_HUMIDITY_BAR_H);
    lv_obj_center(s_humidity_bar);
    lv_bar_set_range(s_humidity_bar, UI_HUMIDITY_MIN_PCT, UI_HUMIDITY_MAX_PCT);
    lv_bar_set_value(s_humidity_bar, GUI_HOME_HUMIDITY_INITIAL_PCT, LV_ANIM_ON);
    lv_obj_set_style_bg_color(s_humidity_bar, lv_color_hex(UI_COLOR_TEAL), LV_PART_INDICATOR);

    s_humidity_label = lv_label_create(container);
    lv_label_set_text(s_humidity_label, "60%\nHumid");
    lv_obj_set_style_text_color(s_humidity_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(s_humidity_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_humidity_label, LV_ALIGN_BOTTOM_MID, 0, GUI_HOME_HUMIDITY_LABEL_Y);
}

static void LVGL_Home_CreateNavButtons(lv_obj_t *parent) {
    lv_obj_t *btn_sensors = lv_btn_create(parent);
    lv_obj_set_size(btn_sensors, GUI_HOME_NAV_BTN_W, GUI_HOME_NAV_BTN_H);
    lv_obj_align(btn_sensors, LV_ALIGN_BOTTOM_LEFT, GUI_HOME_NAV_BTN_MARGIN,
                 -GUI_HOME_NAV_BTN_MARGIN);
    lv_obj_set_style_bg_color(btn_sensors, lv_color_hex(UI_COLOR_ACCENT), 0);
    LVGL_Nav_Attach(btn_sensors, LVGL_SCREEN_SENSORS);

    lv_obj_t *lbl_sensors = lv_label_create(btn_sensors);
    lv_label_set_text(lbl_sensors, LV_SYMBOL_LIST " Sensors");
    lv_obj_center(lbl_sensors);

    lv_obj_t *btn_settings = lv_btn_create(parent);
    lv_obj_set_size(btn_settings, GUI_HOME_NAV_BTN_W, GUI_HOME_NAV_BTN_H);
    lv_obj_align(btn_settings, LV_ALIGN_BOTTOM_RIGHT, -GUI_HOME_NAV_BTN_MARGIN,
                 -GUI_HOME_NAV_BTN_MARGIN);
    lv_obj_set_style_bg_color(btn_settings, lv_color_hex(UI_COLOR_WARN), 0);
    LVGL_Nav_Attach(btn_settings, LVGL_SCREEN_SETTINGS);

    lv_obj_t *lbl_settings = lv_label_create(btn_settings);
    lv_label_set_text(lbl_settings, LV_SYMBOL_SETTINGS " Config");
    lv_obj_center(lbl_settings);
}

void LVGL_Home_Create(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BG), 0);
    LVGL_Screens_Register(LVGL_SCREEN_HOME, screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "STM32F429 Dashboard");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, GUI_HOME_TITLE_Y);

    LVGL_Home_CreateStatusCard(screen);
    LVGL_Home_CreateTemperatureGauge(screen);
    LVGL_Home_CreateHumidityGauge(screen);
    LVGL_Home_CreateNavButtons(screen);
}

void LVGL_Home_SetTemperature(int temp_celsius) {
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

void LVGL_Home_SetHumidity(int humidity_percent) {
    char text[HOME_VALUE_TEXT_LEN];

    humidity_percent = LVGL_Home_Clamp(humidity_percent, UI_HUMIDITY_MIN_PCT, UI_HUMIDITY_MAX_PCT);

    if (s_humidity_bar != NULL) {
        lv_bar_set_value(s_humidity_bar, humidity_percent, LV_ANIM_ON);
    }

    if (s_humidity_label != NULL) {
        lv_snprintf(text, sizeof(text), "%d%%\nHumid", humidity_percent);
        lv_label_set_text(s_humidity_label, text);
    }
}

void LVGL_Home_SetStatus(const char *status) {
    if (s_status_label != NULL && status != NULL) {
        lv_label_set_text(s_status_label, status);
    }
}
