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
 * @brief Bring up the sensor on an already-initialised SPI peripheral.
 * @note  The application owns the SPI (see Peripherals/SPI); this driver never
 *        configures or reconfigures it.
 */
MEMS_StatusTypeDef MEMS_Init(MEMS_HandleTypeDef *hmems, SPI_HandleTypeDef *hspi);

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
