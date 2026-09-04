/**
 ******************************************************************************
 * @file    spi_transfer.h
 * @brief   SPI transfer operations and utilities
 ******************************************************************************
 */

#ifndef SPI_TRANSFER_H
#define SPI_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spi_types.h"

/**
 * @brief   Send bytes to a device, selecting it onto its bus first
 * @param   device Device to talk to
 * @param   pData Bytes to send
 * @param   Size Number of bytes
 * @param   Timeout Timeout in milliseconds
 * @retval  SPI_StatusTypeDef Status of the operation
 */
SPI_StatusTypeDef SPI_Transmit(SPI_Device_t *device, uint8_t *pData, uint16_t Size,
                               uint32_t Timeout);

/**
 * @brief   Read bytes from a device, selecting it onto its bus first
 * @param   device Device to talk to
 * @param   pData Destination buffer
 * @param   Size Number of bytes
 * @param   Timeout Timeout in milliseconds
 * @retval  SPI_StatusTypeDef Status of the operation
 */
SPI_StatusTypeDef SPI_Receive(SPI_Device_t *device, uint8_t *pData, uint16_t Size,
                              uint32_t Timeout);

/**
 * @brief   Full-duplex exchange with a device
 * @param   device Device to talk to
 * @param   pTxData Bytes to send
 * @param   pRxData Destination buffer
 * @param   Size Number of bytes
 * @param   Timeout Timeout in milliseconds
 * @retval  SPI_StatusTypeDef Status of the operation
 */
SPI_StatusTypeDef SPI_TransmitReceive(SPI_Device_t *device, uint8_t *pTxData, uint8_t *pRxData,
                                      uint16_t Size, uint32_t Timeout);

/**
 * @brief   HAL error code from the last transfer on the device's bus
 */
uint32_t SPI_GetError(const SPI_Device_t *device);

/**
 * @brief   Human readable form of a driver status
 */
const char *SPI_GetStatusString(SPI_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* SPI_TRANSFER_H */
