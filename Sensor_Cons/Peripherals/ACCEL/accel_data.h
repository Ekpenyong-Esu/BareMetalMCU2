/**
  ******************************************************************************
  * @file    accel_data.h
  * @brief   Acceleration sample acquisition
  ******************************************************************************
  */

#ifndef ACCEL_DATA_H
#define ACCEL_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "accel_types.h"

/**
 * @brief Read one sample as raw 14-bit counts.
 */
ACCEL_StatusTypeDef ACCEL_ReadRawData(int16_t *xAxis, int16_t *yAxis, int16_t *zAxis);

/**
 * @brief Read one sample as raw counts and g, scaled by the cached range.
 */
ACCEL_StatusTypeDef ACCEL_ReadData(ACCEL_DataTypeDef *data);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_DATA_H */
