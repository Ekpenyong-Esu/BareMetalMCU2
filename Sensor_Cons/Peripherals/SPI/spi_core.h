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

/* Brings up SPI5 with the driver's default settings (master, 8-bit, mode 0). */
SPI_StatusTypeDef SPI_Init(void);
/* Brings up SPI5 using caller-supplied settings instead of the defaults. */
SPI_StatusTypeDef SPI_Init_Custom(const SPI_ConfigTypeDef *config);
/* Tears down the peripheral so it can be re-initialised. */
SPI_StatusTypeDef SPI_DeInit(void);

/* Re-applies the handle's existing Init settings, preserving a custom config that SPI_Init() would overwrite. */
SPI_StatusTypeDef SPI_Reinit(void);

/* Maps a HAL result onto the driver's status enum. */
SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/* Access to the underlying HAL handle, for device drivers built on top of this bus. */
SPI_HandleTypeDef *SPI_GetHandle(void);
/* Whether SPI_Init()/SPI_Init_Custom() has succeeded. */
bool SPI_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* SPI_CORE_H */
