/**
 * @file    eeprom_read.c
 * @brief   EEPROM read operations
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_read.h"
#include "eeprom_core.h"
#include "eeprom_io.h"

/* Private defines -----------------------------------------------------------*/
/* Big-endian byte positions when reassembling multi-byte values */
#define EEPROM_BYTE1_SHIFT 8U
#define EEPROM_BYTE2_SHIFT 16U
#define EEPROM_BYTE3_SHIFT 24U

/* Public functions ----------------------------------------------------------*/

EEPROM_StatusTypeDef EEPROM_Read(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t *data,
                                 uint16_t length) {
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

    return EEPROM_IO_Read(handle, address, data, length);
}

EEPROM_StatusTypeDef EEPROM_ReadByte(EEPROM_HandleTypeDef *handle, uint16_t address,
                                     uint8_t *data) {
    return EEPROM_Read(handle, address, data, 1);
}

EEPROM_StatusTypeDef EEPROM_ReadWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                     uint16_t *data) {
    uint8_t buffer[2] = {0};
    EEPROM_StatusTypeDef status = EEPROM_OK;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, buffer, sizeof(buffer));
    if (status == EEPROM_OK) {
        *data = (uint16_t)(((uint16_t)buffer[0] << EEPROM_BYTE1_SHIFT) | buffer[1]);
    }

    return status;
}

EEPROM_StatusTypeDef EEPROM_ReadDWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                      uint32_t *data) {
    uint8_t buffer[4] = {0};
    EEPROM_StatusTypeDef status = EEPROM_OK;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, buffer, sizeof(buffer));
    if (status == EEPROM_OK) {
        *data = ((uint32_t)buffer[0] << EEPROM_BYTE3_SHIFT) |
                ((uint32_t)buffer[1] << EEPROM_BYTE2_SHIFT) |
                ((uint32_t)buffer[2] << EEPROM_BYTE1_SHIFT) | buffer[3];
    }

    return status;
}

EEPROM_StatusTypeDef EEPROM_ReadFloat(EEPROM_HandleTypeDef *handle, uint16_t address, float *data) {
    union {
        float f;
        uint8_t b[4];
    } conv = {0};
    EEPROM_StatusTypeDef status = EEPROM_OK;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, conv.b, sizeof(conv.b));
    if (status == EEPROM_OK) {
        *data = conv.f;
    }

    return status;
}
