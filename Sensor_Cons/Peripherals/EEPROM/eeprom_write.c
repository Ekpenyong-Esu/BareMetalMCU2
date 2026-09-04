/**
 * @file    eeprom_write.c
 * @brief   EEPROM write operations
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_write.h"
#include "eeprom_core.h"
#include "eeprom_io.h"

/* Public functions ----------------------------------------------------------*/

EEPROM_StatusTypeDef EEPROM_WritePage(EEPROM_HandleTypeDef *handle, uint16_t address,
                                      const uint8_t *data, uint8_t length) {
    EEPROM_StatusTypeDef status = EEPROM_OK;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_CheckRange(handle, address, length);
    if (status != EEPROM_OK) {
        return status;
    }

    if (length == 0) {
        return EEPROM_OK;
    }

    /* A page write that overruns the page wraps to the page start and
       corrupts earlier bytes, so refuse instead of silently truncating. */
    if (((address % handle->config.pageSize) + length) > handle->config.pageSize) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_IO_Write(handle, address, data, length);
    if (status != EEPROM_OK) {
        return status;
    }

    return EEPROM_WaitReady(handle);
}

EEPROM_StatusTypeDef EEPROM_Write(EEPROM_HandleTypeDef *handle, uint16_t address,
                                  const uint8_t *data, uint16_t length) {
    EEPROM_StatusTypeDef status = EEPROM_OK;
    uint16_t pageSize = 0;
    uint16_t bytesWritten = 0;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_CheckRange(handle, address, length);
    if (status != EEPROM_OK) {
        return status;
    }

    pageSize = handle->config.pageSize;

    while (bytesWritten < length) {
        const uint16_t offset = (uint16_t)((address + bytesWritten) % pageSize);
        uint16_t bytesToWrite = (uint16_t)(pageSize - offset);

        if (bytesToWrite > (length - bytesWritten)) {
            bytesToWrite = (uint16_t)(length - bytesWritten);
        }

        status = EEPROM_WritePage(handle, (uint16_t)(address + bytesWritten), data + bytesWritten,
                                  (uint8_t)bytesToWrite);
        if (status != EEPROM_OK) {
            return status;
        }

        bytesWritten = (uint16_t)(bytesWritten + bytesToWrite);
    }

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_WriteByte(EEPROM_HandleTypeDef *handle, uint16_t address,
                                      uint8_t data) {
    return EEPROM_WritePage(handle, address, &data, 1);
}

EEPROM_StatusTypeDef EEPROM_WriteWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                      uint16_t data) {
    const uint8_t buffer[2] = {(uint8_t)(data >> 8), (uint8_t)(data & 0xFFU)};

    return EEPROM_Write(handle, address, buffer, sizeof(buffer));
}

EEPROM_StatusTypeDef EEPROM_WriteDWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                       uint32_t data) {
    const uint8_t buffer[4] = {(uint8_t)(data >> 24), (uint8_t)(data >> 16), (uint8_t)(data >> 8),
                               (uint8_t)(data & 0xFFU)};

    return EEPROM_Write(handle, address, buffer, sizeof(buffer));
}

EEPROM_StatusTypeDef EEPROM_WriteFloat(EEPROM_HandleTypeDef *handle, uint16_t address, float data) {
    union {
        float f;
        uint8_t b[4];
    } conv;

    conv.f = data;

    return EEPROM_Write(handle, address, conv.b, sizeof(conv.b));
}
