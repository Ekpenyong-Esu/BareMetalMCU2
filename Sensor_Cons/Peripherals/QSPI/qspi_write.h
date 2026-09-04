/**
 ******************************************************************************
 * @file    qspi_write.h
 * @brief   Program paths for the serial NOR flash
 ******************************************************************************
 */

#ifndef QSPI_WRITE_H
#define QSPI_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

/**
 * @brief Program up to one page; the transfer must not cross a page boundary.
 */
QSPI_StatusTypeDef QSPI_WritePage(QSPI_HandleStructTypeDef *hqspi, uint32_t address,
                                  const uint8_t *data, uint32_t size);

/**
 * @brief Program an arbitrary span, splitting it across page boundaries.
 */
QSPI_StatusTypeDef QSPI_Write(QSPI_HandleStructTypeDef *hqspi, uint32_t address,
                              const uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_WRITE_H */
