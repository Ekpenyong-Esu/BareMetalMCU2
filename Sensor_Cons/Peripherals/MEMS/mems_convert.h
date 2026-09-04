/**
 ******************************************************************************
 * @file    mems_convert.h
 * @brief   Raw-to-engineering-unit conversion for the L3GD20
 ******************************************************************************
 */

#ifndef MEMS_CONVERT_H
#define MEMS_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

/**
 * @brief Convert a raw axis sample to degrees per second.
 */
float MEMS_ConvertToDPS(int16_t raw_data, MEMS_GyroFullScaleTypeDef full_scale);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_CONVERT_H */
