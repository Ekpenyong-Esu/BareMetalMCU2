/**
 ******************************************************************************
 * @file    flash_read.c
 * @brief   Flash read-back and blank-check operations
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_read.h"
#include "flash_sector.h"
#include <string.h>

/* Public functions ----------------------------------------------------------*/

uint8_t FLASH_ReadByte(uint32_t address) {
    return *(__IO uint8_t *)address;
}

uint16_t FLASH_ReadHalfWord(uint32_t address) {
    return *(__IO uint16_t *)address;
}

uint32_t FLASH_ReadWord(uint32_t address) {
    return *(__IO uint32_t *)address;
}

FLASH_StatusTypeDef FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0U) {
        return FLASH_STATUS_INVALID_PARAM;
    }

    if (!FLASH_IsValidRange(address, length)) {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    memcpy(data, (const void *)address, length);

    return FLASH_STATUS_OK;
}

bool FLASH_IsErased(uint32_t address, uint32_t length) {
    if (!FLASH_IsValidRange(address, length)) {
        return false;
    }

    for (uint32_t i = 0; i < length; i++) {
        if (FLASH_ReadByte(address + i) != FLASH_ERASED_BYTE) {
            return false;
        }
    }

    return true;
}
