/**
  ******************************************************************************
  * @file    qspi_core.h
  * @brief   Lifecycle, identification and power control for the serial NOR flash
  ******************************************************************************
  */

#ifndef QSPI_CORE_H
#define QSPI_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

/* Brings up the QSPI peripheral with the driver's default settings. */
QSPI_StatusTypeDef QSPI_Init(QSPI_HandleStructTypeDef *hqspi);
/* Tears down the peripheral so it can be re-initialised. */
QSPI_StatusTypeDef QSPI_DeInit(QSPI_HandleStructTypeDef *hqspi);
/* Applies caller-supplied settings instead of the defaults. */
QSPI_StatusTypeDef QSPI_Configure(QSPI_HandleStructTypeDef *hqspi, const QSPI_ConfigTypeDef *config);
/* Returns the driver's default QSPI configuration. */
QSPI_ConfigTypeDef QSPI_GetDefaultConfig(void);

/**
 * @brief Issue the two-step software reset sequence (66h, 99h).
 */
QSPI_StatusTypeDef QSPI_Reset(QSPI_HandleStructTypeDef *hqspi);

/* Reads flash size/geometry info detected from the JEDEC ID. */
QSPI_StatusTypeDef QSPI_GetMemoryInfo(QSPI_HandleStructTypeDef *hqspi, QSPI_MemoryInfoTypeDef *memInfo);

/** @param device_id Buffer of at least QSPI_JEDEC_ID_LENGTH bytes. */
QSPI_StatusTypeDef QSPI_ReadID(QSPI_HandleStructTypeDef *hqspi, uint8_t *device_id);

/** @param uniqueID Buffer of at least QSPI_UNIQUE_ID_LENGTH bytes. */
QSPI_StatusTypeDef QSPI_ReadUniqueID(QSPI_HandleStructTypeDef *hqspi, uint8_t *uniqueID);

/* Puts the flash into its lowest-power standby state. */
QSPI_StatusTypeDef QSPI_EnterDeepPowerDown(QSPI_HandleStructTypeDef *hqspi);
/* Wakes the flash from deep power-down. */
QSPI_StatusTypeDef QSPI_ExitDeepPowerDown(QSPI_HandleStructTypeDef *hqspi);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_CORE_H */
