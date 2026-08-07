/**
  ******************************************************************************
  * @file    qspi_status.h
  * @brief   Status register polling and write-enable handling
  ******************************************************************************
  */

#ifndef QSPI_STATUS_H
#define QSPI_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

QSPI_StatusTypeDef QSPI_GetStatus(QSPI_HandleStructTypeDef *hqspi, uint8_t *status);

/**
 * @brief Poll the busy bit until the current program/erase finishes.
 */
QSPI_StatusTypeDef QSPI_WaitForWriteEnd(QSPI_HandleStructTypeDef *hqspi);

/**
 * @brief Issue WREN and confirm the write enable latch actually set.
 */
QSPI_StatusTypeDef QSPI_WriteEnable(QSPI_HandleStructTypeDef *hqspi);
QSPI_StatusTypeDef QSPI_WriteDisable(QSPI_HandleStructTypeDef *hqspi);

/**
 * @brief Run WaitForWriteEnd against a one-shot timeout, restoring the previous one.
 */
QSPI_StatusTypeDef QSPI_WaitForWriteEndWithin(QSPI_HandleStructTypeDef *hqspi, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_STATUS_H */
