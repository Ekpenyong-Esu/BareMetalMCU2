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

SPI_StatusTypeDef SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout);
SPI_StatusTypeDef SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout);
SPI_StatusTypeDef SPI_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);
uint32_t SPI_GetError(void);
const char *SPI_GetStatusString(SPI_StatusTypeDef status);

/**
 * @brief Re-initialise the peripheral after a transfer left it in an error state.
 */
void SPI_RecoverOnError(void);

#ifdef __cplusplus
}
#endif

#endif /* SPI_TRANSFER_H */
