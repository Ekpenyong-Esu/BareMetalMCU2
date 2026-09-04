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

/**
 * @brief Register the part on an open bus, claim its chip select and apply @p config.
 * @param bus    Caller-owned bus, already opened with SPI_BusInit()
 * @param csPort Chip-select port; the driver drives it, the caller picks it
 * @param csPin  Chip-select pin
 * @param config Settings to apply, or NULL for 100 Hz, +/-2g, active mode
 */
ACCEL_StatusTypeDef ACCEL_Init(ACCEL_Handle_t *haccel, SPI_Bus_t *bus, GPIO_TypeDef *csPort,
                               uint16_t csPin, const ACCEL_ConfigTypeDef *config);
ACCEL_StatusTypeDef ACCEL_DeInit(ACCEL_Handle_t *haccel);

/**
 * @brief Verify the device answers with the expected WHO_AM_I value.
 */
ACCEL_StatusTypeDef ACCEL_IsReady(ACCEL_Handle_t *haccel);
ACCEL_StatusTypeDef ACCEL_GetDeviceID(ACCEL_Handle_t *haccel, uint8_t *deviceId);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_CORE_H */
