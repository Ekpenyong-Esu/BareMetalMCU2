/**
 ******************************************************************************
 * @file    spi_transfer.c
 * @brief   SPI transfer operations and utilities
 ******************************************************************************
 */

#include "spi_transfer.h"
#include "spi_core.h"
#include "log.h"

/* A failed transfer can leave the peripheral disabled, so its owner is
   dropped and the next transfer reprograms it for whichever device asks. */
static SPI_StatusTypeDef SPI_RecoverOnError(SPI_Device_t *device, HAL_StatusTypeDef halStatus) {
    SPI_Bus_t *bus = device->bus;

    if (HAL_SPI_DeInit(&bus->hal) != HAL_OK) {
        log_error("SPI: error recovery failed to reset the peripheral");
    }
    bus->owner = NULL;

    return SPI_ConvertHALStatus(halStatus);
}

SPI_StatusTypeDef SPI_Transmit(SPI_Device_t *device, uint8_t *pData, uint16_t Size,
                               uint32_t Timeout) {
    if (pData == NULL || Size == 0) {
        return SPI_INVALID_PARAM;
    }

    SPI_StatusTypeDef status = SPI_Select(device);
    if (status != SPI_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_SPI_Transmit(&device->bus->hal, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return SPI_RecoverOnError(device, halStatus);
    }

    return SPI_OK;
}

SPI_StatusTypeDef SPI_Receive(SPI_Device_t *device, uint8_t *pData, uint16_t Size,
                              uint32_t Timeout) {
    if (pData == NULL || Size == 0) {
        return SPI_INVALID_PARAM;
    }

    SPI_StatusTypeDef status = SPI_Select(device);
    if (status != SPI_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_SPI_Receive(&device->bus->hal, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return SPI_RecoverOnError(device, halStatus);
    }

    return SPI_OK;
}

SPI_StatusTypeDef SPI_TransmitReceive(SPI_Device_t *device, uint8_t *pTxData, uint8_t *pRxData,
                                      uint16_t Size, uint32_t Timeout) {
    if (pTxData == NULL || pRxData == NULL || Size == 0) {
        return SPI_INVALID_PARAM;
    }

    SPI_StatusTypeDef status = SPI_Select(device);
    if (status != SPI_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus =
        HAL_SPI_TransmitReceive(&device->bus->hal, pTxData, pRxData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return SPI_RecoverOnError(device, halStatus);
    }

    return SPI_OK;
}

uint32_t SPI_GetError(const SPI_Device_t *device) {
    SPI_HandleTypeDef *hal = SPI_GetHandle(device);

    return (hal != NULL) ? HAL_SPI_GetError(hal) : HAL_SPI_ERROR_NONE;
}

const char *SPI_GetStatusString(SPI_StatusTypeDef status) {
    switch (status) {
        case SPI_OK:
            return "SPI_OK";
        case SPI_ERROR:
            return "SPI_ERROR";
        case SPI_BUSY:
            return "SPI_BUSY";
        case SPI_TIMEOUT:
            return "SPI_TIMEOUT";
        case SPI_INVALID_PARAM:
            return "SPI_INVALID_PARAM";
        default:
            return "UNKNOWN_STATUS";
    }
}
