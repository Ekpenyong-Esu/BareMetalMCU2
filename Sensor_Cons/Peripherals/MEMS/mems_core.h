/**
 ******************************************************************************
 * @file    mems_core.h
 * @brief   Lifecycle for the L3GD20 MEMS gyroscope driver
 ******************************************************************************
 */

#ifndef MEMS_CORE_H
#define MEMS_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

/**
 * @brief Bring up the sensor on the bus and pins the application chose.
 * @param config Bus, chip select and optional INT pins; bus must already be
 *               open. The SPI settings the L3GD20 needs are fixed by the driver.
 */
MEMS_StatusTypeDef MEMS_Init(MEMS_HandleTypeDef *hmems, const MEMS_Config_t *config);

MEMS_StatusTypeDef MEMS_DeInit(MEMS_HandleTypeDef *hmems);

/**
 * @brief Restore the control registers to their power-on values.
 */
MEMS_StatusTypeDef MEMS_Reset(MEMS_HandleTypeDef *hmems);

/**
 * @brief Fill @p config with the settings MEMS_Init() applies.
 */
MEMS_StatusTypeDef MEMS_GetDefaultConfig(MEMS_GyroConfigTypeDef *config);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_CORE_H */
