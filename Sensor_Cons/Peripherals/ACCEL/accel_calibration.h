/**
  ******************************************************************************
  * @file    accel_calibration.h
  * @brief   Offset calibration for the MMA8452Q
  ******************************************************************************
  */

#ifndef ACCEL_CALIBRATION_H
#define ACCEL_CALIBRATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "accel_types.h"

/**
 * @brief Average a burst of samples and program the resulting offsets.
 * @note  Assumes the board is level and stationary, and that the device is in
 *        the +/-4g range: Z is trimmed towards 8192 counts (1 g).
 */
ACCEL_StatusTypeDef ACCEL_Calibrate(void);

ACCEL_StatusTypeDef ACCEL_SetOffset(int8_t xOffset, int8_t yOffset, int8_t zOffset);
ACCEL_StatusTypeDef ACCEL_GetOffset(int8_t *xOffset, int8_t *yOffset, int8_t *zOffset);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_CALIBRATION_H */
