/**
 ******************************************************************************
 * @file    qspi_io.h
 * @brief   SPI framing primitives for the serial NOR flash
 * @note    These do not touch chip select; the caller owns the frame.
 ******************************************************************************
 */

#ifndef QSPI_IO_H
#define QSPI_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

QSPI_StatusTypeDef QSPI_SendCommand(QSPI_HandleStructTypeDef *hqspi, uint8_t command);

QSPI_StatusTypeDef QSPI_SendCommandWithAddress(QSPI_HandleStructTypeDef *hqspi, uint8_t command,
                                               uint32_t address);

QSPI_StatusTypeDef QSPI_SendData(QSPI_HandleStructTypeDef *hqspi, const uint8_t *data,
                                 uint32_t size);

QSPI_StatusTypeDef QSPI_ReceiveData(QSPI_HandleStructTypeDef *hqspi, uint8_t *data, uint32_t size);

/**
 * @brief Assert CS, send a single opcode, release CS.
 */
QSPI_StatusTypeDef QSPI_SendFramedCommand(QSPI_HandleStructTypeDef *hqspi, uint8_t command);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_IO_H */
