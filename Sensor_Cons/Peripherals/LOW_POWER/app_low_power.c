/**
  ******************************************************************************
  * @file    app_low_power.c
  * @brief   Low power manager lifecycle and the PWR peripheral hooks
  * @details Provides the application overrides for the weak PWR_OptimizeForLowPower
  *          and PWR_RestoreFromLowPower hooks declared in pwr.h.
  ******************************************************************************
  */

#include "app_low_power_core.h"
#include "app_low_power_activity.h"
#include "app_display_power.h"
#include "app_touch_power.h"
#include "app_sdram_power.h"
#include "log.h"
#include "lvgl.h"
#include "lvgl_app.h"
#include "ltdc_panel.h"
#include "ili9341.h"
#include "i2c.h"
#include <lv_port_disp.h>
#include <lv_port_indev.h>

/** Longest we let LVGL run to finish the blanking fade before cutting power. */
#define APP_FADE_DRAIN_TIMEOUT_MS   1000U
#define APP_FADE_DRAIN_STEP_MS      5U

#define DEFAULT_TEMP_VALUE          25
#define DEFAULT_HUMIDITY_VALUE      60

/**
 * @brief  Let the blanking animation complete so its LVGL callbacks release the
 *         overlay before the SDRAM and LTDC it draws into are powered down.
 */
static void APP_DrainFade(void)
{
    uint32_t wait_start = HAL_GetTick();

    while (APP_Display_IsFading() &&
           ((HAL_GetTick() - wait_start) < APP_FADE_DRAIN_TIMEOUT_MS)) {
        lv_timer_handler();
        HAL_Delay(APP_FADE_DRAIN_STEP_MS);
    }
}

/**
 * @brief  Gate the clocks of everything this application does not use while idle.
 * @note   Must stay in step with APP_EnablePeripheralClocks().
 */
static void APP_DisablePeripheralClocks(void)
{
    __HAL_RCC_TIM2_CLK_DISABLE();
    __HAL_RCC_TIM3_CLK_DISABLE();
    __HAL_RCC_TIM4_CLK_DISABLE();
    __HAL_RCC_TIM5_CLK_DISABLE();

    __HAL_RCC_USART2_CLK_DISABLE();
    __HAL_RCC_USART3_CLK_DISABLE();
    __HAL_RCC_USART6_CLK_DISABLE();

    __HAL_RCC_SPI2_CLK_DISABLE();
    __HAL_RCC_SPI3_CLK_DISABLE();

    __HAL_RCC_ADC1_CLK_DISABLE();
    __HAL_RCC_ADC2_CLK_DISABLE();
    __HAL_RCC_ADC3_CLK_DISABLE();

    /* I2C3 and its EXTI stay clocked: the touch INT is the wake source. */
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_GPIOF_CLK_DISABLE();
    __HAL_RCC_GPIOG_CLK_DISABLE();
    __HAL_RCC_GPIOH_CLK_DISABLE();
    __HAL_RCC_GPIOI_CLK_DISABLE();
}

static void APP_EnablePeripheralClocks(void)
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_TIM5_CLK_ENABLE();

    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_SPI3_CLK_ENABLE();

    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC2_CLK_ENABLE();
    __HAL_RCC_ADC3_CLK_ENABLE();
}

static void APP_RestoreGUIState(void)
{
    log_debug("APP: Restoring GUI state");

    /* LVGL_App exposes only setters, so there is nothing to snapshot on the way
       down; the UI comes back up showing its defaults. */
    LVGL_App_Init();
    LVGL_App_UpdateTemperature(DEFAULT_TEMP_VALUE);
    LVGL_App_UpdateHumidity(DEFAULT_HUMIDITY_VALUE);
    LVGL_App_UpdateStatus("System Active");
}

PWR_StatusTypeDef APP_LowPowerInit(void)
{
    APP_ActivityReset();
    APP_Display_Init();

    /* Wakeup pins are delegated to the touchscreen driver (TS_ITConfig during TS_Init). */
    log_debug("APP: Wakeup pins delegated to touchscreen driver");
    log_debug("APP: Low power management initialized for STM32F429I-DISC1");

    return PWR_OK;
}

PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals)
{
    log_debug("APP: Optimizing STM32F429I-DISC1 for low power (keep: %d)", keepPeripherals);

    if (keepPeripherals) {
        return PWR_OK;
    }

    APP_Display_PowerOff();
    APP_DrainFade();

    APP_Touch_PowerOff();
    APP_SDRAM_PowerOff();

    APP_DisablePeripheralClocks();

    log_debug("APP: Non-critical peripherals disabled");

    return PWR_OK;
}

PWR_StatusTypeDef PWR_RestoreFromLowPower(void)
{
    PWR_StatusTypeDef status = PWR_OK;

    log_debug("APP: Restoring STM32F429I-DISC1 from low power");

    PWR_EnableHighPerformance();
    PWR_ClearStandbyFlag();

    APP_EnablePeripheralClocks();

    if (!APP_SDRAM_PowerOn()) {
        status = PWR_ERROR;
    }

    /* Re-init the panel and LTDC before waking them: after a deep sleep their
       registers are gone, so APP_Display_PowerOn() would drive dead hardware. */
    ili9341_Init();

    if (LTDC_HW_Init() != HAL_OK) {
        log_error("APP: LTDC re-initialization failed");
        status = PWR_ERROR;
    }

    APP_Display_PowerOn();

    if (!APP_Touch_PowerOn()) {
        status = PWR_ERROR;
    }

    /* lv_init() is self-guarding, but the port inits are not: running them on an
       already-initialized LVGL registers a second display and input device. */
    if (!lv_is_initialized()) {
        lv_init();
        lv_port_disp_init();
        lv_port_indev_init();
    }

    APP_RestoreGUIState();
    APP_ActivityReset();

    LVGL_App_UpdateStatus("System Resumed");

    log_debug("APP: Restoration completed");

    return status;
}
