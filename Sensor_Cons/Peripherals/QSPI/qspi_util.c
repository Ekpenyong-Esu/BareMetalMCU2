/**
 ******************************************************************************
 * @file    qspi_util.c
 * @brief   Address arithmetic and status strings
 ******************************************************************************
 */

#include "qspi_util.h"

static const char *const s_statusStrings[] = {
    [QSPI_OK] = "OK",
    [QSPI_ERROR] = "Error",
    [QSPI_BUSY] = "Busy",
    [QSPI_TIMEOUT] = "Timeout",
    [QSPI_INVALID_PARAM] = "Invalid Parameter",
    [QSPI_NOT_SUPPORTED] = "Not Supported",
    [QSPI_WRITE_PROTECTED] = "Write Protected",
    [QSPI_ERASE_ERROR] = "Erase Error",
    [QSPI_PROGRAM_ERROR] = "Program Error",
};

bool QSPI_IsAddressValid(uint32_t address, uint32_t size) {
    /* Compared this way so address + size cannot wrap past the end of the flash. */
    return (size <= QSPI_FLASH_SIZE_BYTES) && (address <= (QSPI_FLASH_SIZE_BYTES - size));
}

uint32_t QSPI_GetSectorAddress(uint32_t address) {
    return address & ~(QSPI_SECTOR_SIZE - 1U);
}

uint32_t QSPI_GetBlockAddress(uint32_t address) {
    return address & ~(QSPI_BLOCK_SIZE - 1U);
}

const char *QSPI_GetStatusString(QSPI_StatusTypeDef status) {
    if ((size_t)status >= (sizeof(s_statusStrings) / sizeof(s_statusStrings[0]))) {
        return "Unknown";
    }

    return s_statusStrings[status];
}
