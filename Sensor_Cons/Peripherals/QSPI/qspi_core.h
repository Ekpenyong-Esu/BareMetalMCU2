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

/**
 * @brief Register the flash on the caller's bus, claim its chip select and
 *        identify the part.
 * @param config Bus (already opened with SPI_BusInit), chip-select pin and
 *               clock divider; start from QSPI_GetDefaultConfig().
 */
QSPI_StatusTypeDef QSPI_Init(QSPI_HandleStructTypeDef *hqspi, const QSPI_ConfigTypeDef *config);

/**
 * @brief Put the flash to sleep and release the chip select; the bus is left open.
 */
QSPI_StatusTypeDef QSPI_DeInit(QSPI_HandleStructTypeDef *hqspi);

/**
 * @brief Change the clock divider of an initialised flash.
 * @note  Bus and chip select are ignored here; they are fixed at QSPI_Init().
 */
QSPI_StatusTypeDef QSPI_Configure(QSPI_HandleStructTypeDef *hqspi,
                                  const QSPI_ConfigTypeDef *config);

/**
 * @brief Default clock divider with the wiring left blank for the caller.
 */
QSPI_ConfigTypeDef QSPI_GetDefaultConfig(void);

/**
 * @brief Issue the two-step software reset sequence (66h, 99h).
 */
QSPI_StatusTypeDef QSPI_Reset(QSPI_HandleStructTypeDef *hqspi);

QSPI_StatusTypeDef QSPI_GetMemoryInfo(QSPI_HandleStructTypeDef *hqspi,
                                      QSPI_MemoryInfoTypeDef *memInfo);

/** @param device_id Buffer of at least QSPI_JEDEC_ID_LENGTH bytes. */
QSPI_StatusTypeDef QSPI_ReadID(QSPI_HandleStructTypeDef *hqspi, uint8_t *device_id);

/** @param uniqueID Buffer of at least QSPI_UNIQUE_ID_LENGTH bytes. */
QSPI_StatusTypeDef QSPI_ReadUniqueID(QSPI_HandleStructTypeDef *hqspi, uint8_t *uniqueID);

QSPI_StatusTypeDef QSPI_EnterDeepPowerDown(QSPI_HandleStructTypeDef *hqspi);
QSPI_StatusTypeDef QSPI_ExitDeepPowerDown(QSPI_HandleStructTypeDef *hqspi);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_CORE_H */
