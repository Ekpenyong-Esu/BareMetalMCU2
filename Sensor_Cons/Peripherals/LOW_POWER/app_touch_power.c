/**
  ******************************************************************************
  * @file    app_touch_power.c
  * @brief   Touchscreen bus power state, keeping the wake interrupt alive
  ******************************************************************************
  */

#include "app_touch_power.h"
#include "log.h"
#include "i2c.h"
#include "ts_types.h"

static bool touchscreen_is_active = true;

void APP_Touch_PowerOff(void)
{
    if (!touchscreen_is_active) {
        return;
    }

    log_debug("APP: Turning touchscreen off (keeping INT enabled)");

    /* The EXTI line and its NVIC entry stay armed: it is the only wake source.
       Only the I2C bus is released, so callbacks must be ignored until it is back. */
    I2C_DeInit();

    touchscreen_is_active = false;
}

bool APP_Touch_PowerOn(void)
{
    if (touchscreen_is_active) {
        return true;
    }

    log_debug("APP: Turning touchscreen on");

    if (I2C_Init() != I2C_OK) {
        log_error("APP: touchscreen I2C re-initialization failed");
        return false;
    }

    /* Drop the edge that woke us, otherwise the callback fires before the bus
       has settled. */
    __HAL_GPIO_EXTI_CLEAR_IT(TS_INT_PIN);
    HAL_NVIC_EnableIRQ(TS_INT_EXTI_IRQn);

    touchscreen_is_active = true;

    return true;
}

bool APP_Touch_IsActive(void)
{
    return touchscreen_is_active;
}
