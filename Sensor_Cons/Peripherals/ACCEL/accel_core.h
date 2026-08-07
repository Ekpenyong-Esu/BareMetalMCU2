/**
  ******************************************************************************
  * @file    accel_core.h
  * @brief   Lifecycle and identity for the MMA8452Q accelerometer
  ******************************************************************************
  */

#ifndef ACCEL_CORE_H
#define ACCEL_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "accel_types.h"

ACCEL_StatusTypeDef ACCEL_Init(void);
ACCEL_StatusTypeDef ACCEL_Init_Custom(const ACCEL_ConfigTypeDef *config);
ACCEL_StatusTypeDef ACCEL_DeInit(void);

/**
 * @brief Verify the device answers with the expected WHO_AM_I value.
 */
ACCEL_StatusTypeDef ACCEL_IsReady(void);
ACCEL_StatusTypeDef ACCEL_GetDeviceID(uint8_t *deviceId);

/**
 * @brief Last range written to the device; used to scale samples without an
 *        extra bus transaction per read.
 */
uint8_t ACCEL_GetCachedRange(void);
void    ACCEL_CacheRange(uint8_t range);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_CORE_H */
