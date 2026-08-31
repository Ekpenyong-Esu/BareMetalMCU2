/**
  ******************************************************************************
  * @file    accel_io.h
  * @brief   Register-level SPI access for the MMA8452Q
  ******************************************************************************
  */

#ifndef ACCEL_IO_H
#define ACCEL_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "accel_types.h"

/** Largest burst read the driver performs (X/Y/Z MSB+LSB). */
#define ACCEL_BURST_MAX          6U

/**
 * @brief Claim a slot on the shared SPI bus; call before any register access.
 */
ACCEL_StatusTypeDef ACCEL_IO_Init(void);

ACCEL_StatusTypeDef ACCEL_WriteRegister(uint8_t reg, uint8_t value);
ACCEL_StatusTypeDef ACCEL_ReadRegister(uint8_t reg, uint8_t *value);

/**
 * @brief Read up to ACCEL_BURST_MAX consecutive registers.
 */
ACCEL_StatusTypeDef ACCEL_ReadRegisters(uint8_t reg, uint8_t *buffer, uint16_t length);

/**
 * @brief Read-modify-write: clear the bits in @p mask, then set the bits in @p value.
 */
ACCEL_StatusTypeDef ACCEL_UpdateRegister(uint8_t reg, uint8_t mask, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_IO_H */
