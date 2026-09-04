/**
 ******************************************************************************
 * @file    qspi_io.c
 * @brief   SPI framing primitives for the serial NOR flash
 ******************************************************************************
 */

#include "qspi_io.h"
#include "qspi_flash.h"
#include "qspi_hw.h"
#include "spi_transfer.h"

/* 24-bit addresses go out MSB first */
#define QSPI_ADDRESS_BYTE2_SHIFT 16U
#define QSPI_ADDRESS_BYTE1_SHIFT 8U

/* SPI_Transmit/Receive count in uint16_t, so longer buffers are chunked;
   passing a uint32_t straight through silently truncated the transfer. */
static QSPI_StatusTypeDef QSPI_Transfer(QSPI_HandleStructTypeDef *hqspi, uint8_t *buffer,
                                        uint32_t size, bool receive) {
    while (size > 0U) {
        uint16_t chunk = (size > QSPI_HAL_MAX_TRANSFER) ? QSPI_HAL_MAX_TRANSFER : (uint16_t)size;
        SPI_StatusTypeDef status =
            receive ? SPI_Receive(&hqspi->device, buffer, chunk, hqspi->Timeout)
                    : SPI_Transmit(&hqspi->device, buffer, chunk, hqspi->Timeout);

        if (status != SPI_OK) {
            return QSPI_ERROR;
        }

        buffer += chunk;
        size -= chunk;
    }

    return QSPI_OK;
}

QSPI_StatusTypeDef QSPI_SendCommand(QSPI_HandleStructTypeDef *hqspi, uint8_t command) {
    return QSPI_Transfer(hqspi, &command, 1U, false);
}

QSPI_StatusTypeDef QSPI_SendCommandWithAddress(QSPI_HandleStructTypeDef *hqspi, uint8_t command,
                                               uint32_t address) {
    uint8_t frame[1U + QSPI_ADDRESS_BYTES];

    frame[0] = command;
    frame[1] = (uint8_t)(address >> QSPI_ADDRESS_BYTE2_SHIFT);
    frame[2] = (uint8_t)(address >> QSPI_ADDRESS_BYTE1_SHIFT);
    frame[3] = (uint8_t)address;

    return QSPI_Transfer(hqspi, frame, sizeof(frame), false);
}

QSPI_StatusTypeDef QSPI_SendData(QSPI_HandleStructTypeDef *hqspi, const uint8_t *data,
                                 uint32_t size) {
    return QSPI_Transfer(hqspi, (uint8_t *)data, size, false);
}

QSPI_StatusTypeDef QSPI_ReceiveData(QSPI_HandleStructTypeDef *hqspi, uint8_t *data, uint32_t size) {
    return QSPI_Transfer(hqspi, data, size, true);
}

QSPI_StatusTypeDef QSPI_SendFramedCommand(QSPI_HandleStructTypeDef *hqspi, uint8_t command) {
    QSPI_ChipSelect(hqspi, true);
    QSPI_StatusTypeDef status = QSPI_SendCommand(hqspi, command);
    QSPI_ChipSelect(hqspi, false);

    return status;
}
