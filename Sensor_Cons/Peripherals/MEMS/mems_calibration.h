/**
  ******************************************************************************
  * @file    mems_calibration.h
  * @brief   Gyroscope zero-rate offset calibration
  ******************************************************************************
  */

#ifndef MEMS_CALIBRATION_H
#define MEMS_CALIBRATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

/**
 * @brief Average @p samples readings at rest and store the result as the offset.
 * @param samples Pass 0 to use the default sample count.
 */
MEMS_StatusTypeDef MEMS_CalibrateGyroscope(MEMS_HandleTypeDef *hmems, uint16_t samples);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_CALIBRATION_H */
