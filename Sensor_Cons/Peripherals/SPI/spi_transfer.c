/**
  ******************************************************************************
  * @file    spi_transfer.c
  * @brief   SPI transfer operations and utilities
  ******************************************************************************
  */

#include "spi_transfer.h"
#include "spi_core.h"
#include "log.h"

void SPI_RecoverOnError(void)
{
    SPI_DeInit();

    if (SPI_Reinit() != SPI_OK) {
        log_error("SPI: error recovery failed to re-initialize SPI5");
    }
}

SPI_StatusTypeDef SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || Size == 0) {
        return SPI_INVALID_PARAM;
    }

    halStatus = HAL_SPI_Transmit(SPI_GetHandle(), pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        SPI_RecoverOnError();
        return SPI_ConvertHALStatus(halStatus);
    }

    return SPI_OK;
}

SPI_StatusTypeDef SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || Size == 0) {
        return SPI_INVALID_PARAM;
    }

    halStatus = HAL_SPI_Receive(SPI_GetHandle(), pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        SPI_RecoverOnError();
        return SPI_ConvertHALStatus(halStatus);
    }

    return SPI_OK;
}

SPI_StatusTypeDef SPI_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pTxData == NULL || pRxData == NULL || Size == 0) {
        return SPI_INVALID_PARAM;
    }

    halStatus = HAL_SPI_TransmitReceive(SPI_GetHandle(), pTxData, pRxData, Size, Timeout);
    if (halStatus != HAL_OK) {
        SPI_RecoverOnError();
        return SPI_ConvertHALStatus(halStatus);
    }

    return SPI_OK;
}

uint32_t SPI_GetError(void)
{
    return HAL_SPI_GetError(SPI_GetHandle());
}

const char *SPI_GetStatusString(SPI_StatusTypeDef status)
{
    switch (status) {
        case SPI_OK:           return "SPI_OK";
        case SPI_ERROR:        return "SPI_ERROR";
        case SPI_BUSY:         return "SPI_BUSY";
        case SPI_TIMEOUT:      return "SPI_TIMEOUT";
        case SPI_INVALID_PARAM: return "SPI_INVALID_PARAM";
        default:               return "UNKNOWN_STATUS";
    }
}
