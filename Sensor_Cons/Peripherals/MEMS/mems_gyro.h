/**
  ******************************************************************************
  * @file    mems_gyro.h
  * @brief   Gyroscope configuration and sample acquisition
  ******************************************************************************
  */

#ifndef MEMS_GYRO_H
#define MEMS_GYRO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

/* Applies output data rate, scale and filter settings. */
MEMS_StatusTypeDef MEMS_GyroConfig(MEMS_HandleTypeDef *hmems, const MEMS_GyroConfigTypeDef *config);
/* Reads the raw (unscaled) X/Y/Z sample. */
MEMS_StatusTypeDef MEMS_GyroReadRaw(MEMS_HandleTypeDef *hmems, MEMS_AxesRawTypeDef *axes);
/* Reads X/Y/Z scaled to degrees-per-second. */
MEMS_StatusTypeDef MEMS_GyroRead(MEMS_HandleTypeDef *hmems, MEMS_AxesTypeDef *axes);
/* Selects which axes are active. */
MEMS_StatusTypeDef MEMS_SetAxisEnable(MEMS_HandleTypeDef *hmems, uint8_t axis_mask);
MEMS_StatusTypeDef MEMS_GetFullScale(MEMS_HandleTypeDef *hmems, MEMS_GyroFullScaleTypeDef *full_scale);
/* Enables/disables the device's low-power power-down mode. */
MEMS_StatusTypeDef MEMS_SetPowerMode(MEMS_HandleTypeDef *hmems, bool power_down);

/**
 * @brief Same as MEMS_GyroConfig but without the "driver ready" guard.
 * @note  For MEMS_Init(), which must configure the device before the handle is
 *        marked ready.
 */
MEMS_StatusTypeDef MEMS_GyroApplyConfig(MEMS_HandleTypeDef *hmems, const MEMS_GyroConfigTypeDef *config);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_GYRO_H */
