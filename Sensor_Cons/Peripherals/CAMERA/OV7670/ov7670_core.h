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
 * @param busConfig SCCB bus settings to use, or NULL for I2C_ConfigDefault().
 * @note  DCMI stays owned by the application; the SCCB port is registered here
 *        so a slower device sharing I2C3 cannot impose its clock on the camera.
 */
OV7670_StatusTypeDef OV7670_Init(OV7670_Handle_t *hov7670,
                                 DCMI_HandleTypeDef *hdcmi,
                                 const I2C_ConfigTypeDef *busConfig);

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
