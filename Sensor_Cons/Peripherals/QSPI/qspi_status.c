/**
 ******************************************************************************
 * @file    qspi_status.c
 * @brief   Status register polling and write-enable handling
 ******************************************************************************
 */

#include "qspi_status.h"
#include "qspi_flash.h"
#include "qspi_hw.h"
#include "qspi_io.h"

#define QSPI_POLL_INTERVAL_MS 1U

QSPI_StatusTypeDef QSPI_GetStatus(QSPI_HandleStructTypeDef *hqspi, uint8_t *status) {
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK || status == NULL) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_ChipSelect(hqspi, true);

    QSPI_StatusTypeDef result = QSPI_SendCommand(hqspi, QSPI_CMD_READ_STATUS_REG);
    if (result == QSPI_OK) {
        result = QSPI_ReceiveData(hqspi, status, QSPI_STATUS_REG_SIZE);
    }

    QSPI_ChipSelect(hqspi, false);

    return result;
}

QSPI_StatusTypeDef QSPI_WaitForWriteEnd(QSPI_HandleStructTypeDef *hqspi) {
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    const uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < hqspi->Timeout) {
        uint8_t status = 0;

        if (QSPI_GetStatus(hqspi, &status) != QSPI_OK) {
            return QSPI_ERROR;
        }

        if ((status & QSPI_SR_BUSY) == 0U) {
            return QSPI_OK;
        }

        HAL_Delay(QSPI_POLL_INTERVAL_MS);
    }

    return QSPI_TIMEOUT;
}

QSPI_StatusTypeDef QSPI_WaitForWriteEndWithin(QSPI_HandleStructTypeDef *hqspi, uint32_t timeout) {
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    const uint32_t previous = hqspi->Timeout;

    hqspi->Timeout = timeout;
    QSPI_StatusTypeDef status = QSPI_WaitForWriteEnd(hqspi);
    hqspi->Timeout = previous;

    return status;
}

QSPI_StatusTypeDef QSPI_WriteEnable(QSPI_HandleStructTypeDef *hqspi) {
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    QSPI_StatusTypeDef status = QSPI_SendFramedCommand(hqspi, QSPI_CMD_WRITE_ENABLE);
    if (status != QSPI_OK) {
        return status;
    }

    const uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < QSPI_WRITE_ENABLE_TIMEOUT) {
        uint8_t statusReg = 0;

        if ((QSPI_GetStatus(hqspi, &statusReg) == QSPI_OK) && (statusReg & QSPI_SR_WEL)) {
            return QSPI_OK;
        }

        HAL_Delay(QSPI_POLL_INTERVAL_MS);
    }

    return QSPI_ERROR;
}

QSPI_StatusTypeDef QSPI_WriteDisable(QSPI_HandleStructTypeDef *hqspi) {
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    return QSPI_SendFramedCommand(hqspi, QSPI_CMD_WRITE_DISABLE);
}
