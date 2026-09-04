/**
 ******************************************************************************
 * @file    qspi_util.h
 * @brief   Address arithmetic and status strings
 ******************************************************************************
 */

#ifndef QSPI_UTIL_H
#define QSPI_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

bool QSPI_IsAddressValid(uint32_t address, uint32_t size);
uint32_t QSPI_GetSectorAddress(uint32_t address);
uint32_t QSPI_GetBlockAddress(uint32_t address);
const char *QSPI_GetStatusString(QSPI_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_UTIL_H */
