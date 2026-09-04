/**
 ******************************************************************************
 * @file    accel_convert.h
 * @brief   Raw-sample decoding and unit conversion for the MMA8452Q
 ******************************************************************************
 */

#ifndef ACCEL_CONVERT_H
#define ACCEL_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "accel_types.h"

/**
 * @brief Combine a register pair into a sign-extended 14-bit sample.
 */
int16_t ACCEL_ConvertRawToSigned(uint8_t msb, uint8_t lsb);

/**
 * @brief Scale a raw sample to g.
 * @param range One of ACCEL_RANGE_*; anything else is treated as +/-2g.
 */
float ACCEL_ConvertToG(int16_t raw, uint8_t range);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_CONVERT_H */
