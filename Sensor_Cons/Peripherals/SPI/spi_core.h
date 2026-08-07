/**
  ******************************************************************************
  * @file    spi_core.h
  * @brief   Lifecycle and handle accessor for the SPI bus driver
  ******************************************************************************
  */

#ifndef SPI_CORE_H
#define SPI_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spi_types.h"

SPI_StatusTypeDef SPI_Init(void);
SPI_StatusTypeDef SPI_Init_Custom(const SPI_ConfigTypeDef *config);
SPI_StatusTypeDef SPI_DeInit(void);

/**
 * @brief Re-apply the handle's existing Init settings, keeping a custom
 *        configuration that SPI_Init() would overwrite with the defaults.
 */
SPI_StatusTypeDef SPI_Reinit(void);

/**
 * @brief Map a HAL result onto the driver's status enum.
 */
SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus);

SPI_HandleTypeDef *SPI_GetHandle(void);
bool SPI_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* SPI_CORE_H */
