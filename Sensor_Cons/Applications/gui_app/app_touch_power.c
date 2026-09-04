/**
 ******************************************************************************
 * @file    app_touch_power.c
 * @brief   Touchscreen bus power state, keeping the wake interrupt alive
 ******************************************************************************
 */

#include "app_touch_power.h"
#include "gui_board.h"
#include "log.h"
#include "i2c.h"

static bool touchscreen_is_active = true;

void APP_Touch_PowerOff(void) {
    if (!touchscreen_is_active) {
        return;
    }

    log_debug("APP: Turning touchscreen off (keeping INT enabled)");

    /* The EXTI line and its NVIC entry stay armed: it is the only wake source.
       Only the I2C bus is released, so callbacks must be ignored until it is back. */
    I2C_BusDeInit(GUI_Board_TouchBus());

    touchscreen_is_active = false;
}

bool APP_Touch_PowerOn(void) {
    if (touchscreen_is_active) {
        return true;
    }

    log_debug("APP: Turning touchscreen on");

    /* The controller kept its registers while the bus was closed, and the bus
       reprograms itself for whichever device transfers first, so reopening
       the wires is all that is needed. */
    if (!GUI_Board_TouchBusOpen()) {
        return false;
    }

    /* Drop the edge that woke us, otherwise the callback fires before the bus
       has settled. */
    __HAL_GPIO_EXTI_CLEAR_IT(GUI_BOARD_TOUCH_INT_PIN);
    HAL_NVIC_EnableIRQ(GUI_BOARD_TOUCH_INT_IRQn);

    touchscreen_is_active = true;

    return true;
}

bool APP_Touch_IsActive(void) {
    return touchscreen_is_active;
}
