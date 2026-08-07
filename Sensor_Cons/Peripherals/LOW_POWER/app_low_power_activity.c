/**
  ******************************************************************************
  * @file    app_low_power_activity.c
  * @brief   User activity tracking and the inactivity policy it drives
  ******************************************************************************
  */

#include "app_low_power_activity.h"
#include "app_display_power.h"
#include "app_touch_power.h"
#include "log.h"

static uint32_t last_activity_time = 0;
static uint32_t last_touch_tick = 0;

/* Unsigned subtraction keeps this correct across the 49-day tick wrap. */
static uint32_t APP_InactiveTime(void)
{
    return HAL_GetTick() - last_activity_time;
}

void APP_ActivityReset(void)
{
    last_activity_time = HAL_GetTick();
    last_touch_tick = last_activity_time - APP_TOUCH_ACTIVITY_DEBOUNCE_MS;
}

void APP_UpdateActivity(void)
{
    last_activity_time = HAL_GetTick();
}

void APP_TouchActivity(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - last_touch_tick) < APP_TOUCH_ACTIVITY_DEBOUNCE_MS) {
        log_debug("APP: Touch activity ignored (debounce)");
        return;
    }

    last_touch_tick = now;
    last_activity_time = now;

    log_debug("APP: Touch activity detected");

    if (APP_Display_IsDimmed()) {
        APP_Display_Dim(false);
    }

    if (!APP_Display_IsOn()) {
        APP_Display_PowerOn();
    }

    if (!APP_Touch_IsActive()) {
        (void)APP_Touch_PowerOn();
    }
}

bool APP_ShouldEnterLowPower(void)
{
    uint32_t inactive_time = APP_InactiveTime();

    if (!APP_Display_IsOn()) {
        return true;
    }

    /* Later deadline first: dimming happens on the way to power-off, so testing
       the dim state here would permanently block the power-off branch. */
    if (inactive_time >= APP_DISPLAY_OFF_TIMEOUT_MS) {
        APP_Display_PowerOff();
        return false;
    }

    if (!APP_Display_IsDimmed() && inactive_time >= APP_DISPLAY_DIM_TIMEOUT_MS) {
        APP_Display_Dim(true);
        return false;
    }

    return (inactive_time >= APP_LOW_POWER_TIMEOUT_MS);
}

PWR_StatusTypeDef APP_EnterLowPowerMode(void)
{
    uint32_t inactive_time = APP_InactiveTime();
    PWR_LowPowerConfigTypeDef config;

    PWR_GetDefaultLowPowerConfig(&config);

    if (inactive_time < APP_LOW_POWER_SHORT_MS) {
        config.mode = PWR_LOW_POWER_MODE_LIGHT;
        config.keepPeripherals = true;
    } else {
        config.mode = PWR_LOW_POWER_MODE_DEEP;
        config.keepPeripherals = false;
    }

    /* The touch EXTI is the only wake source, so no PWR wakeup pin is armed. */
    config.wakeupSources = PWR_SRC_NONE;
    config.wakeupTimeMs = APP_LOW_POWER_SHORT_MS;
    config.optimizeVoltage = true;

    log_debug("APP: Entering low power mode after %lu ms inactivity",
              (unsigned long)inactive_time);

    return PWR_EnterLowPowerMode(&config);
}
