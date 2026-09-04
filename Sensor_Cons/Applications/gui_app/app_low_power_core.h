/**
 ******************************************************************************
 * @file    app_low_power_core.h
 * @brief   Low power manager lifecycle
 ******************************************************************************
 */

#ifndef APP_LOW_POWER_CORE_H
#define APP_LOW_POWER_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_low_power_types.h"
#include "pwr.h"

/**
 * @brief Initialize activity monitoring and the display power controls.
 * @note  The touch driver owns the wake configuration, so TS_Init() must have
 *        run before the first low power entry.
 */
PWR_StatusTypeDef APP_LowPowerInit(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_LOW_POWER_CORE_H */
