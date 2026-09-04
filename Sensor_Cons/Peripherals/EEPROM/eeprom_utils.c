/**
 * @file    eeprom_utils.c
 * @brief   EEPROM bulk erase, verification and self test
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_utils.h"
#include "eeprom_core.h"
#include "eeprom_read.h"
#include "eeprom_write.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define EEPROM_ERASED_VALUE 0xFFU
/** Largest span EEPROM_EraseRange can express in its uint16_t length */
#define EEPROM_ERASE_STEP 0x8000U

/* Public functions ----------------------------------------------------------*/

EEPROM_StatusTypeDef EEPROM_EraseRange(EEPROM_HandleTypeDef *handle, uint16_t startAddress,
                                       uint16_t length) {
    uint8_t eraseBuffer[EEPROM_CHUNK_SIZE];
    EEPROM_StatusTypeDef status = EEPROM_OK;
    uint16_t bytesErased = 0;

    status = EEPROM_CheckRange(handle, startAddress, length);
    if (status != EEPROM_OK) {
        return status;
    }

    memset(eraseBuffer, EEPROM_ERASED_VALUE, sizeof(eraseBuffer));

    while (bytesErased < length) {
        uint16_t bytesToErase = (uint16_t)(length - bytesErased);

        if (bytesToErase > sizeof(eraseBuffer)) {
            bytesToErase = sizeof(eraseBuffer);
        }

        status =
            EEPROM_Write(handle, (uint16_t)(startAddress + bytesErased), eraseBuffer, bytesToErase);
        if (status != EEPROM_OK) {
            return status;
        }

        bytesErased = (uint16_t)(bytesErased + bytesToErase);
    }

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_Erase(EEPROM_HandleTypeDef *handle) {
    EEPROM_StatusTypeDef status = EEPROM_CheckRange(handle, 0, 0);
    uint32_t total = 0;
    uint32_t done = 0;

    if (status != EEPROM_OK) {
        return status;
    }

    /* A 64 KB device does not fit EraseRange's uint16_t length, so walk it */
    total = handle->config.totalSize;

    while (done < total) {
        uint32_t chunk = total - done;

        if (chunk > EEPROM_ERASE_STEP) {
            chunk = EEPROM_ERASE_STEP;
        }

        status = EEPROM_EraseRange(handle, (uint16_t)done, (uint16_t)chunk);
        if (status != EEPROM_OK) {
            return status;
        }

        done += chunk;
    }

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_Verify(EEPROM_HandleTypeDef *handle, uint16_t address,
                                   const uint8_t *data, uint16_t length) {
    uint8_t readBuffer[EEPROM_CHUNK_SIZE];
    EEPROM_StatusTypeDef status = EEPROM_OK;
    uint16_t bytesVerified = 0;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_CheckRange(handle, address, length);
    if (status != EEPROM_OK) {
        return status;
    }

    while (bytesVerified < length) {
        uint16_t bytesToVerify = (uint16_t)(length - bytesVerified);

        if (bytesToVerify > sizeof(readBuffer)) {
            bytesToVerify = sizeof(readBuffer);
        }

        status =
            EEPROM_Read(handle, (uint16_t)(address + bytesVerified), readBuffer, bytesToVerify);
        if (status != EEPROM_OK) {
            return status;
        }

        if (memcmp(readBuffer, data + bytesVerified, bytesToVerify) != 0) {
            return EEPROM_ERROR;
        }

        bytesVerified = (uint16_t)(bytesVerified + bytesToVerify);
    }

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_Test(EEPROM_HandleTypeDef *handle, uint16_t testAddress) {
    const uint8_t testPattern[] = {0xAA, 0x55, 0x00, 0xFF};
    uint8_t originalData[sizeof(testPattern)] = {0};
    EEPROM_StatusTypeDef status = EEPROM_OK;

    status = EEPROM_CheckRange(handle, testAddress, sizeof(testPattern));
    if (status != EEPROM_OK) {
        return status;
    }

    status = EEPROM_Read(handle, testAddress, originalData, sizeof(originalData));
    if (status != EEPROM_OK) {
        return status;
    }

    status = EEPROM_Write(handle, testAddress, testPattern, sizeof(testPattern));
    if (status != EEPROM_OK) {
        return status;
    }

    status = EEPROM_Verify(handle, testAddress, testPattern, sizeof(testPattern));

    /* Always put the caller's data back, even when the check failed */
    if (status == EEPROM_OK) {
        status = EEPROM_Write(handle, testAddress, originalData, sizeof(originalData));
    }
    else {
        (void)EEPROM_Write(handle, testAddress, originalData, sizeof(originalData));
    }

    return status;
}
