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

ACCEL_StatusTypeDef ACCEL_SetMode(ACCEL_Handle_t *haccel, uint8_t mode);
ACCEL_StatusTypeDef ACCEL_GetMode(ACCEL_Handle_t *haccel, uint8_t *mode);

ACCEL_StatusTypeDef ACCEL_SetDataRate(ACCEL_Handle_t *haccel, uint8_t odr);

/**
 * @brief Write the range and remember it in the handle for sample scaling.
 */
ACCEL_StatusTypeDef ACCEL_SetRange(ACCEL_Handle_t *haccel, uint8_t range);
ACCEL_StatusTypeDef ACCEL_GetRange(ACCEL_Handle_t *haccel, uint8_t *range);

ACCEL_StatusTypeDef ACCEL_EnableHighPassFilter(ACCEL_Handle_t *haccel, bool enable);
ACCEL_StatusTypeDef ACCEL_EnableLowNoise(ACCEL_Handle_t *haccel, bool enable);

ACCEL_StatusTypeDef ACCEL_ConfigInterrupts(ACCEL_Handle_t *haccel,
                                           const ACCEL_IntConfigTypeDef *config);
ACCEL_StatusTypeDef ACCEL_GetInterruptSource(ACCEL_Handle_t *haccel, uint8_t *intSource);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_CONFIG_H */
