/**
  ******************************************************************************
  * @file    accel_config.h
  * @brief   Runtime configuration of the MMA8452Q
  ******************************************************************************
  */

#ifndef ACCEL_CONFIG_H
#define ACCEL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "accel_types.h"

ACCEL_StatusTypeDef ACCEL_SetMode(uint8_t mode);
ACCEL_StatusTypeDef ACCEL_GetMode(uint8_t *mode);

ACCEL_StatusTypeDef ACCEL_SetDataRate(uint8_t odr);

ACCEL_StatusTypeDef ACCEL_SetRange(uint8_t range);
ACCEL_StatusTypeDef ACCEL_GetRange(uint8_t *range);

ACCEL_StatusTypeDef ACCEL_EnableHighPassFilter(bool enable);
ACCEL_StatusTypeDef ACCEL_EnableLowNoise(bool enable);

ACCEL_StatusTypeDef ACCEL_ConfigInterrupts(const ACCEL_IntConfigTypeDef *config);
ACCEL_StatusTypeDef ACCEL_GetInterruptSource(uint8_t *intSource);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_CONFIG_H */
