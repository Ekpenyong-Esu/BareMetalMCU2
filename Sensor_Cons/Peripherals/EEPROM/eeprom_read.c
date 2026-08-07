/**
 * @file    eeprom_read.c
 * @brief   EEPROM read operations
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_read.h"
#include "eeprom_core.h"
#include "eeprom_io.h"

/* Public functions ----------------------------------------------------------*/

EEPROM_StatusTypeDef EEPROM_Read(EEPROM_HandleTypeDef* handle,
                                 uint16_t address, uint8_t* data, uint16_t length)
{
    EEPROM_StatusTypeDef status;

    if (data == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_CheckRange(handle, address, length);
    if (status != EEPROM_OK)
    {
        return status;
    }

    if (length == 0)
    {
        return EEPROM_OK;
    }

    return EEPROM_IO_Read(handle, address, data, length);
}

EEPROM_StatusTypeDef EEPROM_ReadByte(EEPROM_HandleTypeDef* handle,
                                     uint16_t address, uint8_t* data)
{
    return EEPROM_Read(handle, address, data, 1);
}

EEPROM_StatusTypeDef EEPROM_ReadWord(EEPROM_HandleTypeDef* handle,
                                     uint16_t address, uint16_t* data)
{
    uint8_t buffer[2] = {0};
    EEPROM_StatusTypeDef status;

    if (data == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, buffer, sizeof(buffer));
    if (status == EEPROM_OK)
    {
        *data = (uint16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);
    }

    return status;
}

EEPROM_StatusTypeDef EEPROM_ReadDWord(EEPROM_HandleTypeDef* handle,
                                      uint16_t address, uint32_t* data)
{
    uint8_t buffer[4] = {0};
    EEPROM_StatusTypeDef status;

    if (data == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, buffer, sizeof(buffer));
    if (status == EEPROM_OK)
    {
        *data = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) |
                ((uint32_t)buffer[2] << 8) | buffer[3];
    }

    return status;
}

EEPROM_StatusTypeDef EEPROM_ReadFloat(EEPROM_HandleTypeDef* handle,
                                      uint16_t address, float* data)
{
    union {
        float f;
        uint8_t b[4];
    } conv = {0};
    EEPROM_StatusTypeDef status;

    if (data == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, conv.b, sizeof(conv.b));
    if (status == EEPROM_OK)
    {
        *data = conv.f;
    }

    return status;
}
