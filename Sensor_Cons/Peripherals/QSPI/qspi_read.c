/**
 ******************************************************************************
 * @file    qspi_read.c
 * @brief   Read paths for the serial NOR flash
 ******************************************************************************
 */

#include "qspi_read.h"
#include "qspi_flash.h"
#include "qspi_hw.h"
#include "qspi_io.h"
#include "qspi_util.h"

static QSPI_StatusTypeDef QSPI_ReadWithCommand(QSPI_HandleStructTypeDef *hqspi, uint8_t command,
                                               uint32_t address, uint8_t *data, uint32_t size,
                                               uint8_t dummyBytes) {
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK || data == NULL) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, size)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_ChipSelect(hqspi, true);

    QSPI_StatusTypeDef status = QSPI_SendCommandWithAddress(hqspi, command, address);

    if (status == QSPI_OK && dummyBytes > 0U) {
        const uint8_t dummy[1] = {0x00U};

        for (uint8_t i = 0; (i < dummyBytes) && (status == QSPI_OK); i++) {
            status = QSPI_SendData(hqspi, dummy, sizeof(dummy));
        }
    }

    if (status == QSPI_OK) {
        status = QSPI_ReceiveData(hqspi, data, size);
    }

    QSPI_ChipSelect(hqspi, false);

    return status;
}

QSPI_StatusTypeDef QSPI_Read(QSPI_HandleStructTypeDef *hqspi, uint32_t address, uint8_t *data,
                             uint32_t size) {
    return QSPI_ReadWithCommand(hqspi, QSPI_CMD_READ_DATA, address, data, size, 0U);
}

QSPI_StatusTypeDef QSPI_FastRead(QSPI_HandleStructTypeDef *hqspi, uint32_t address, uint8_t *data,
                                 uint32_t size) {
    return QSPI_ReadWithCommand(hqspi, QSPI_CMD_FAST_READ, address, data, size, 1U);
}
