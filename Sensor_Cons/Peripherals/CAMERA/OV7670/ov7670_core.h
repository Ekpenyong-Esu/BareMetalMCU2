/**
  ******************************************************************************
  * @file    ov7670_core.h
  * @brief   Lifecycle for the OV7670 camera driver
  ******************************************************************************
  */

#ifndef OV7670_CORE_H
#define OV7670_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ov7670_types.h"

/**
 * @brief Detect the sensor, reset it and apply the default configuration.
 * @note  The I2C and DCMI peripherals are owned by the application; this driver
 *        only uses the handles it is given.
 */
OV7670_StatusTypeDef OV7670_Init(OV7670_Handle_t *hov7670,
                                 I2C_HandleTypeDef *hi2c,
                                 DCMI_HandleTypeDef *hdcmi);

OV7670_StatusTypeDef OV7670_DeInit(OV7670_Handle_t *hov7670);

OV7670_StatusTypeDef OV7670_Config(OV7670_Handle_t *hov7670, const OV7670_Config_t *config);

/**
 * @brief Software reset; leaves the sensor in its power-on register state.
 */
OV7670_StatusTypeDef OV7670_Reset(OV7670_Handle_t *hov7670);

/**
 * @brief Fill @p config with the settings OV7670_Init() applies.
 */
OV7670_StatusTypeDef OV7670_GetDefaultConfig(OV7670_Config_t *config);

OV7670_StatusTypeDef OV7670_GetChipID(OV7670_Handle_t *hov7670, uint16_t *chip_id);
OV7670_StatusTypeDef OV7670_GetStatus(OV7670_Handle_t *hov7670);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_CORE_H */
