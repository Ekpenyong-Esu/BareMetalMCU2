/**
  ******************************************************************************
  * @file    crc_core.h
  * @brief   Lifecycle, calculation and status for the CRC driver
  ******************************************************************************
  */

#ifndef CRC_CORE_H
#define CRC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "crc_types.h"

HAL_StatusTypeDef CRC_Init(const CRC_Config *config);
HAL_StatusTypeDef CRC_DeInit(void);
HAL_StatusTypeDef CRC_Calculate(const uint8_t *data, uint32_t size, uint32_t *crc);
HAL_StatusTypeDef CRC_Calculate32(const uint32_t *data, uint32_t size, uint32_t *crc);
HAL_StatusTypeDef CRC_Accumulate(const uint8_t *data, uint32_t size, uint32_t *crc);
HAL_StatusTypeDef CRC_Reset(void);
HAL_StatusTypeDef CRC_GetStatus(CRC_Status *status);
HAL_StatusTypeDef CRC_SetPolynomial(uint32_t polynomial);
HAL_StatusTypeDef CRC_SetInitValue(uint32_t init_value);
CRC_HandleTypeDef *CRC_GetHandle(void);
void CRC_GetDefaultConfig(CRC_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* CRC_CORE_H */
