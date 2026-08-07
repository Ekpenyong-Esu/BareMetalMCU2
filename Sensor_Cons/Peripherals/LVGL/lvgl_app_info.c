/*******************************************************************************
 * LVGL Application - System info screen
 ******************************************************************************/

#include "lvgl_app_info.h"
#include "lvgl_app_nav.h"
#include "lvgl_app_screens.h"
#include "lvgl_app_types.h"
#include "lvgl.h"

static const char *const s_info_text =
    "MCU: STM32F429ZI\n"
    "Core: ARM Cortex-M4\n"
    "Freq: 168 MHz\n"
    "Flash: 2 MB\n"
    "RAM: 256 KB\n"
    "Display: 240x320\n"
    "LVGL: v9.4.x\n"
    "\n"
    "Status: Running";

void LVGL_Info_Create(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BG), 0);
    LVGL_Screens_Register(LVGL_SCREEN_INFO, screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, LV_SYMBOL_CALL " System Information");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *card = lv_obj_create(screen);
    lv_obj_set_size(card, 200, 200);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(UI_COLOR_CARD), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(UI_COLOR_BORDER), 0);
    lv_obj_set_style_border_width(card, 2, 0);

    lv_obj_t *text = lv_label_create(card);
    lv_label_set_text_static(text, s_info_text);
    lv_obj_set_style_text_color(text, lv_color_hex(UI_COLOR_ACCENT), 0);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_14, 0);
    lv_obj_center(text);

    (void)LVGL_Nav_CreateBackButton(screen, LVGL_SCREEN_SETTINGS);
}
