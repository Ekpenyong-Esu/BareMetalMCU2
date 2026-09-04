/*******************************************************************************
 * LVGL Application Types - screen identifiers and UI palette
 ******************************************************************************/

#ifndef LVGL_APP_TYPES_H
#define LVGL_APP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Screen identifiers
 *---------------------------------------------------------------------------*/
typedef enum {
    LVGL_SCREEN_HOME = 0,
    LVGL_SCREEN_SENSORS,
    LVGL_SCREEN_SETTINGS,
    LVGL_SCREEN_INFO,
    LVGL_SCREEN_COUNT
} LVGL_ScreenId_t;

/*-----------------------------------------------------------------------------
 * UI palette
 *---------------------------------------------------------------------------*/
#define UI_COLOR_BG 0x1a1a2e      /* Screen background */
#define UI_COLOR_CARD 0x16213e    /* Card / container background */
#define UI_COLOR_BORDER 0x0f4c75  /* Card border */
#define UI_COLOR_ACCENT 0x3be477  /* Primary accent (green) */
#define UI_COLOR_WARN 0xf7b731    /* Secondary accent (amber) */
#define UI_COLOR_TEAL 0x4ecdc4    /* Humidity / volume */
#define UI_COLOR_TEMP 0xff6b6b    /* Temperature gauge */
#define UI_COLOR_INFO 0xa29bfe    /* Info button */
#define UI_COLOR_NEUTRAL 0x5f6368 /* Back buttons */

/*-----------------------------------------------------------------------------
 * Shared value ranges (the gauges clamp to these, so the labels must too)
 *---------------------------------------------------------------------------*/
#define UI_TEMP_MIN_C 0
#define UI_TEMP_MAX_C 100
#define UI_HUMIDITY_MIN_PCT 0
#define UI_HUMIDITY_MAX_PCT 100
#define UI_CHART_MIN 0
#define UI_CHART_MAX 100

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_TYPES_H */
