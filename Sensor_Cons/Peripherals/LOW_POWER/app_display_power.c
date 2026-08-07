/**
  ******************************************************************************
  * @file    app_display_power.c
  * @brief   Backlight, panel and LTDC power states with fade transitions
  ******************************************************************************
  */

#include "app_display_power.h"
#include "log.h"
#include "lvgl.h"
#include "ltdc_core.h"
#include "ili9341.h"

#define APP_FADE_DIM_MS                  300U
#define APP_FADE_POWER_MS                500U

/* Backlight sits on GPIOK, which is outside the GPIO driver's A-I range. */
#define LCD_BL_GPIO_Port                 GPIOK
#define LCD_BL_Pin                       GPIO_PIN_3

static bool display_is_on = true;
static bool display_is_dimmed = false;

/* Non-NULL only while a fade animation is running */
static lv_obj_t *s_dim_overlay = NULL;

static void APP_DimAnimExec(void *var, int32_t value)
{
    lv_obj_t *overlay = (lv_obj_t *)var;

    if (overlay != NULL) {
        lv_obj_set_style_bg_opa(overlay, (lv_opa_t)value, 0);
    }
}

static void APP_DropOverlay(lv_anim_t *anim)
{
    lv_obj_t *overlay = (lv_obj_t *)anim->var;

    if (overlay != NULL) {
        lv_obj_del(overlay);
        if (s_dim_overlay == overlay) {
            s_dim_overlay = NULL;
        }
    }
}

static void APP_DimAnimReady_Remove(lv_anim_t *anim)
{
    APP_DropOverlay(anim);
}

static void APP_DimAnimReady_Dim(lv_anim_t *anim)
{
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
    display_is_dimmed = true;

    log_debug("APP: Display dimmed");

    APP_DropOverlay(anim);
}

static void APP_DimAnimReady_PowerOff(lv_anim_t *anim)
{
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);

    ili9341_SleepIn();

    if (LTDC_GetHandle()->Instance != NULL) {
        __HAL_LTDC_DISABLE(LTDC_GetHandle());
    }

    display_is_on = false;
    display_is_dimmed = false;

    APP_DropOverlay(anim);
}

static void APP_FadeToBlack(uint32_t msec, bool power_off_after)
{
    lv_anim_t anim;

    if (s_dim_overlay != NULL) {
        return;
    }

    s_dim_overlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_dim_overlay);
    lv_obj_set_size(s_dim_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(s_dim_overlay, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(s_dim_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_dim_overlay, LV_OPA_TRANSP, 0);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, s_dim_overlay);
    lv_anim_set_exec_cb(&anim, APP_DimAnimExec);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&anim, (msec != 0U) ? msec : APP_FADE_POWER_MS);
    lv_anim_set_ready_cb(&anim, power_off_after ? APP_DimAnimReady_PowerOff : APP_DimAnimReady_Dim);
    lv_anim_start(&anim);
}

static void APP_FadeFromBlack(uint32_t msec)
{
    lv_anim_t anim;

    /* Restore brightness unconditionally; only the animation needs an overlay. */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    display_is_dimmed = false;

    if (s_dim_overlay == NULL) {
        return;
    }

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, s_dim_overlay);
    lv_anim_set_exec_cb(&anim, APP_DimAnimExec);
    lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&anim, (msec != 0U) ? msec : APP_FADE_DIM_MS);
    lv_anim_set_ready_cb(&anim, APP_DimAnimReady_Remove);
    lv_anim_start(&anim);
}

void APP_Display_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOK_CLK_ENABLE();
    GPIO_InitStruct.Pin = LCD_BL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

    display_is_on = true;
    display_is_dimmed = false;
}

void APP_Display_PowerOff(void)
{
    if (!display_is_on) {
        return;
    }

    log_debug("APP: Turning display off");
    APP_FadeToBlack(APP_FADE_POWER_MS, true);
}

void APP_Display_PowerOn(void)
{
    if (display_is_on) {
        return;
    }

    log_debug("APP: Turning display on");

    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

    ili9341_SleepOut();

    if (LTDC_GetHandle()->Instance != NULL) {
        __HAL_LTDC_ENABLE(LTDC_GetHandle());
    }

    display_is_on = true;
    display_is_dimmed = false;

    APP_FadeFromBlack(APP_FADE_DIM_MS);
}

void APP_Display_Dim(bool dim)
{
    if (dim && !display_is_dimmed) {
        log_debug("APP: Dimming display");
        APP_FadeToBlack(APP_FADE_DIM_MS, false);
    } else if (!dim && display_is_dimmed) {
        log_debug("APP: Restoring display brightness");
        APP_FadeFromBlack(APP_FADE_DIM_MS);
    }
}

bool APP_Display_IsOn(void)
{
    return display_is_on;
}

bool APP_Display_IsDimmed(void)
{
    return display_is_dimmed;
}

bool APP_Display_IsFading(void)
{
    return (s_dim_overlay != NULL);
}
