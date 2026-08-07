/**
  ******************************************************************************
  * @file    app_touch_power.h
  * @brief   Touchscreen bus power state, keeping the wake interrupt alive
  * @note    Internal to the low power manager; not part of app_low_power.h.
  ******************************************************************************
  */

#ifndef APP_TOUCH_POWER_H
#define APP_TOUCH_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_low_power_types.h"

/**
 * @brief Release the I2C bus while leaving the touch EXTI configured, so the
 *        controller can still wake the MCU.
 */
void APP_Touch_PowerOff(void);

/**
 * @brief Bring the I2C bus back up and re-arm the touch interrupt.
 * @retval false if the bus could not be re-initialized.
 */
bool APP_Touch_PowerOn(void);

bool APP_Touch_IsActive(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TOUCH_POWER_H */
