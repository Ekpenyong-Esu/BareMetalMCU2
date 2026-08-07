/**
  ******************************************************************************
  * @file    i2c_core.h
  * @brief   Lifecycle and handle accessor for the I2C bus driver
  ******************************************************************************
  */

#ifndef I2C_CORE_H
#define I2C_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_types.h"

I2C_StatusTypeDef I2C_Init(void);
I2C_StatusTypeDef I2C_Init_Custom(const I2C_ConfigTypeDef *config);
I2C_StatusTypeDef I2C_DeInit(void);

/**
 * @brief Re-apply the handle's existing Init settings, keeping a custom
 *        configuration that I2C_Init() would overwrite with the defaults.
 */
I2C_StatusTypeDef I2C_Reinit(void);

/**
 * @brief Map a HAL result onto the driver's status enum.
 */
I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/**
 * @brief Access the HAL handle (for consumers that need to pass it to their own init).
 */
I2C_HandleTypeDef *I2C_GetHandle(void);

/**
 * @brief True if the bus has been initialised and the handle is usable.
 */
bool I2C_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_CORE_H */
