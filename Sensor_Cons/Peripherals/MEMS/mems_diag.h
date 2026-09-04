/**
 ******************************************************************************
 * @file    mems_diag.h
 * @brief   Device identity, status, temperature and self-test
 ******************************************************************************
 */

#ifndef MEMS_DIAG_H
#define MEMS_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

MEMS_StatusTypeDef MEMS_GetDeviceInfo(MEMS_HandleTypeDef *hmems, MEMS_DeviceInfoTypeDef *info);
MEMS_StatusTypeDef MEMS_ReadStatus(MEMS_HandleTypeDef *hmems, uint8_t *status_reg);
MEMS_StatusTypeDef MEMS_ReadTemperature(MEMS_HandleTypeDef *hmems, float *temperature);
MEMS_StatusTypeDef MEMS_SelfTest(MEMS_HandleTypeDef *hmems, bool *result);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_DIAG_H */
