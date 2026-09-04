/**
 ******************************************************************************
 * @file    qspi_read.h
 * @brief   Read paths for the serial NOR flash
 ******************************************************************************
 */

#ifndef QSPI_READ_H
#define QSPI_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

QSPI_StatusTypeDef QSPI_Read(QSPI_HandleStructTypeDef *hqspi, uint32_t address, uint8_t *data,
                             uint32_t size);

/**
 * @brief Same as QSPI_Read but uses the 0x0B opcode with one dummy byte,
 *        which the flash accepts at a higher clock.
 */
QSPI_StatusTypeDef QSPI_FastRead(QSPI_HandleStructTypeDef *hqspi, uint32_t address, uint8_t *data,
                                 uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_READ_H */
