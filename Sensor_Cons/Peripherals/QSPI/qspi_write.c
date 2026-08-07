/**
  ******************************************************************************
  * @file    qspi_write.c
  * @brief   Program paths for the serial NOR flash
  ******************************************************************************
  */

#include "qspi_write.h"
#include "qspi_flash.h"
#include "qspi_hw.h"
#include "qspi_io.h"
#include "qspi_status.h"
#include "qspi_util.h"

QSPI_StatusTypeDef QSPI_WritePage(QSPI_HandleStructTypeDef *hqspi, uint32_t address,
                                  const uint8_t *data, uint32_t size)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK || data == NULL) {
        return QSPI_INVALID_PARAM;
    }

    if (size == 0U || size > QSPI_PAGE_SIZE || !QSPI_IsAddressValid(address, size)) {
        return QSPI_INVALID_PARAM;
    }

    if (((address % QSPI_PAGE_SIZE) + size) > QSPI_PAGE_SIZE) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status = QSPI_WriteEnable(hqspi);
    if (status != QSPI_OK) {
        return status;
    }

    QSPI_ChipSelect(true);

    status = QSPI_SendCommandWithAddress(hqspi, QSPI_CMD_PAGE_PROGRAM, address);
    if (status == QSPI_OK) {
        status = QSPI_SendData(hqspi, data, size);
    }

    QSPI_ChipSelect(false);

    if (status != QSPI_OK) {
        return status;
    }

    return (QSPI_WaitForWriteEnd(hqspi) == QSPI_OK) ? QSPI_OK : QSPI_PROGRAM_ERROR;
}

QSPI_StatusTypeDef QSPI_Write(QSPI_HandleStructTypeDef *hqspi, uint32_t address,
                              const uint8_t *data, uint32_t size)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK || data == NULL) {
        return QSPI_INVALID_PARAM;
    }

    if (size == 0U || !QSPI_IsAddressValid(address, size)) {
        return QSPI_INVALID_PARAM;
    }

    while (size > 0U) {
        const uint32_t pageRoom = QSPI_PAGE_SIZE - (address % QSPI_PAGE_SIZE);
        const uint32_t chunk = (pageRoom > size) ? size : pageRoom;

        QSPI_StatusTypeDef status = QSPI_WritePage(hqspi, address, data, chunk);
        if (status != QSPI_OK) {
            return status;
        }

        address += chunk;
        data += chunk;
        size -= chunk;
    }

    return QSPI_OK;
}
