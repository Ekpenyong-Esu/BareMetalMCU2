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

/* Brings up the CRC peripheral with the given polynomial/init settings. */
HAL_StatusTypeDef CRC_Init(const CRC_Config *config);
/* Tears down the peripheral so it can be re-initialised. */
HAL_StatusTypeDef CRC_DeInit(void);
/* One-shot CRC over a byte buffer; resets the running value first. */
HAL_StatusTypeDef CRC_Calculate(const uint8_t *data, uint32_t size, uint32_t *crc);
/* One-shot CRC over a 32-bit-word buffer; resets the running value first. */
HAL_StatusTypeDef CRC_Calculate32(const uint32_t *data, uint32_t size, uint32_t *crc);
/* Feeds more bytes into the running CRC without resetting it. */
HAL_StatusTypeDef CRC_Accumulate(const uint8_t *data, uint32_t size, uint32_t *crc);
/* Clears the running CRC back to its initial value. */
HAL_StatusTypeDef CRC_Reset(void);
/* Reports whether the peripheral is initialised and its last computed value. */
HAL_StatusTypeDef CRC_GetStatus(CRC_Status *status);
/* Changes the polynomial used for future calculations. */
HAL_StatusTypeDef CRC_SetPolynomial(uint32_t polynomial);
/* Changes the seed value future calculations start from. */
HAL_StatusTypeDef CRC_SetInitValue(uint32_t init_value);
/* Access to the underlying HAL handle. */
CRC_HandleTypeDef *CRC_GetHandle(void);
/* Fills a config struct with the driver's default polynomial/settings. */
void CRC_GetDefaultConfig(CRC_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* CRC_CORE_H */
