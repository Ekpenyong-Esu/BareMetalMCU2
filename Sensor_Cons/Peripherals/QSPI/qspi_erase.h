/**
  ******************************************************************************
  * @file    qspi_erase.h
  * @brief   Erase paths for the serial NOR flash
  ******************************************************************************
  */

#ifndef QSPI_ERASE_H
#define QSPI_ERASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

QSPI_StatusTypeDef QSPI_EraseSector(QSPI_HandleStructTypeDef *hqspi, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseBlock32K(QSPI_HandleStructTypeDef *hqspi, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseBlock64K(QSPI_HandleStructTypeDef *hqspi, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseChip(QSPI_HandleStructTypeDef *hqspi);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_ERASE_H */
