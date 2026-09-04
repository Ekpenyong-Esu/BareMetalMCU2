/**
 ******************************************************************************
 * @file    qspi_erase.c
 * @brief   Erase paths for the serial NOR flash
 ******************************************************************************
 */

#include "qspi_erase.h"
#include "qspi_flash.h"
#include "qspi_hw.h"
#include "qspi_io.h"
#include "qspi_status.h"
#include "qspi_util.h"

/* Every erase is the same sequence; only the opcode, whether an address is
   sent, and how long the part may take differ. */
static QSPI_StatusTypeDef QSPI_EraseWithCommand(QSPI_HandleStructTypeDef *hqspi, uint8_t command,
                                                uint32_t address, bool addressed,
                                                uint32_t timeout) {
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    if (addressed && !QSPI_IsAddressValid(address, 1U)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status = QSPI_WriteEnable(hqspi);
    if (status != QSPI_OK) {
        return status;
    }

    QSPI_ChipSelect(hqspi, true);
    status = addressed ? QSPI_SendCommandWithAddress(hqspi, command, address)
                       : QSPI_SendCommand(hqspi, command);
    QSPI_ChipSelect(hqspi, false);

    if (status != QSPI_OK) {
        return status;
    }

    return (QSPI_WaitForWriteEndWithin(hqspi, timeout) == QSPI_OK) ? QSPI_OK : QSPI_ERASE_ERROR;
}

QSPI_StatusTypeDef QSPI_EraseSector(QSPI_HandleStructTypeDef *hqspi, uint32_t address) {
    return QSPI_EraseWithCommand(hqspi, QSPI_CMD_SECTOR_ERASE, address, true, QSPI_ERASE_TIMEOUT);
}

QSPI_StatusTypeDef QSPI_EraseBlock32K(QSPI_HandleStructTypeDef *hqspi, uint32_t address) {
    return QSPI_EraseWithCommand(hqspi, QSPI_CMD_BLOCK_ERASE_32K, address, true,
                                 QSPI_ERASE_TIMEOUT);
}

QSPI_StatusTypeDef QSPI_EraseBlock64K(QSPI_HandleStructTypeDef *hqspi, uint32_t address) {
    return QSPI_EraseWithCommand(hqspi, QSPI_CMD_BLOCK_ERASE_64K, address, true,
                                 QSPI_ERASE_TIMEOUT);
}

QSPI_StatusTypeDef QSPI_EraseChip(QSPI_HandleStructTypeDef *hqspi) {
    return QSPI_EraseWithCommand(hqspi, QSPI_CMD_CHIP_ERASE, 0U, false, QSPI_CHIP_ERASE_TIMEOUT);
}
