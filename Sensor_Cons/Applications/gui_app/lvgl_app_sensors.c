/*******************************************************************************
 * LVGL Application - Sensor monitoring screen
 *******************************************************************************
 * Live line chart plus a row of four sensor value cards.
 ******************************************************************************/

#include "lvgl_app_sensors.h"
#include "lvgl_app_nav.h"
#include "lvgl_app_screens.h"
#include "lvgl_app_types.h"
#include "lvgl.h"

#define SENSOR_CHART_POINTS 10
#define SENSOR_CARD_COUNT 4
#define SENSOR_CARD_SIZE 50
#define SENSOR_CARD_PITCH 55
#define SENSOR_CARD_ORIGIN_X 10

/* Layout constants (pixels) */
#define GUI_SENSORS_TITLE_Y 10        /* Title offset from the top edge */
#define GUI_SENSORS_CHART_W 200       /* Chart width */
#define GUI_SENSORS_CHART_H 120       /* Chart height */
#define GUI_SENSORS_CHART_Y (-20)     /* Chart offset from the screen centre */
#define GUI_SENSORS_CARD_BOTTOM (-50) /* Card row offset from the bottom edge */

typedef struct {
    const char *name;
    const char *value;
    uint32_t color;
} SensorCard_t;

static const SensorCard_t s_sensor_cards[SENSOR_CARD_COUNT] = {
    {"Accel", "1.2g", UI_COLOR_TEMP},
    {"Gyro", "45/s", UI_COLOR_TEAL},
    {"Temp", "25C", UI_COLOR_WARN},
    {"Press", "1013", UI_COLOR_INFO},
};

static const int32_t s_chart_seed[SENSOR_CHART_POINTS] = {10, 20, 35, 45, 50, 55, 60, 70, 65, 50};

static lv_obj_t *s_chart;
static lv_chart_series_t *s_chart_series;

static void LVGL_Sensors_CreateChart(lv_obj_t *parent) {
    s_chart = lv_chart_create(parent);
    lv_obj_set_size(s_chart, GUI_SENSORS_CHART_W, GUI_SENSORS_CHART_H);
    lv_obj_align(s_chart, LV_ALIGN_CENTER, 0, GUI_SENSORS_CHART_Y);
    lv_chart_set_type(s_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, UI_CHART_MIN, UI_CHART_MAX);
    lv_obj_set_style_bg_color(s_chart, lv_color_hex(UI_COLOR_CARD), 0);

    s_chart_series =
        lv_chart_add_series(s_chart, lv_color_hex(UI_COLOR_ACCENT), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(s_chart, SENSOR_CHART_POINTS);

    if (s_chart_series != NULL) {
        for (int i = 0; i < SENSOR_CHART_POINTS; i++) {
            lv_chart_set_next_value(s_chart, s_chart_series, s_chart_seed[i]);
        }
    }
}

static void LVGL_Sensors_CreateCards(lv_obj_t *parent) {
    for (int i = 0; i < SENSOR_CARD_COUNT; i++) {
        lv_obj_t *card = lv_obj_create(parent);
        lv_obj_set_size(card, SENSOR_CARD_SIZE, SENSOR_CARD_SIZE);
        lv_obj_align(card, LV_ALIGN_BOTTOM_LEFT, SENSOR_CARD_ORIGIN_X + (i * SENSOR_CARD_PITCH),
                     GUI_SENSORS_CARD_BOTTOM);
        lv_obj_set_style_bg_color(card, lv_color_hex(s_sensor_cards[i].color), 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_set_style_pad_all(card, 0, 0);

        lv_obj_t *name = lv_label_create(card);
        lv_label_set_text(name, s_sensor_cards[i].name);
        lv_obj_set_style_text_font(name, &lv_font_montserrat_14, 0);
        lv_obj_align(name, LV_ALIGN_TOP_MID, 0, 0);

        lv_obj_t *value = lv_label_create(card);
        lv_label_set_text(value, s_sensor_cards[i].value);
        lv_obj_set_style_text_font(value, &lv_font_montserrat_14, 0);
        lv_obj_align(value, LV_ALIGN_BOTTOM_MID, 0, 0);
    }
}

void LVGL_Sensors_Create(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BG), 0);
    LVGL_Screens_Register(LVGL_SCREEN_SENSORS, screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, LV_SYMBOL_EYE_OPEN " Sensor Monitor");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, GUI_SENSORS_TITLE_Y);

    LVGL_Sensors_CreateChart(screen);
    LVGL_Sensors_CreateCards(screen);

    (void)LVGL_Nav_CreateBackButton(screen, LVGL_SCREEN_HOME);
}

void LVGL_Sensors_AddChartData(int value) {
    if (s_chart == NULL || s_chart_series == NULL) {
        return;
    }

    if (value < UI_CHART_MIN) {
        value = UI_CHART_MIN;
    }
    else if (value > UI_CHART_MAX) {
        value = UI_CHART_MAX;
    }

    lv_chart_set_next_value(s_chart, s_chart_series, value);
}
