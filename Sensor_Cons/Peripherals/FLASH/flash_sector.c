/**
 ******************************************************************************
 * @file    flash_sector.c
 * @brief   Flash sector geometry and address range validation
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_sector.h"

/* Private variables ---------------------------------------------------------*/

/**
 * @brief Start address of every sector, plus the end of Flash as a terminator
 * @note  Sector sizes are derived from consecutive entries, so this table is
 *        the single source of truth for the STM32F429 (2 MB) geometry.
 */
static const uint32_t FLASH_SECTOR_ADDRESSES[FLASH_SECTOR_TOTAL + 1U] = {
    /* Bank 1 */
    0x08000000U, /* Sector 0:  16KB */
    0x08004000U, /* Sector 1:  16KB */
    0x08008000U, /* Sector 2:  16KB */
    0x0800C000U, /* Sector 3:  16KB */
    0x08010000U, /* Sector 4:  64KB */
    0x08020000U, /* Sector 5:  128KB */
    0x08040000U, /* Sector 6:  128KB */
    0x08060000U, /* Sector 7:  128KB */
    0x08080000U, /* Sector 8:  128KB */
    0x080A0000U, /* Sector 9:  128KB */
    0x080C0000U, /* Sector 10: 128KB */
    0x080E0000U, /* Sector 11: 128KB */
    /* Bank 2 */
    0x08100000U, /* Sector 12: 16KB */
    0x08104000U, /* Sector 13: 16KB */
    0x08108000U, /* Sector 14: 16KB */
    0x0810C000U, /* Sector 15: 16KB */
    0x08110000U, /* Sector 16: 64KB */
    0x08120000U, /* Sector 17: 128KB */
    0x08140000U, /* Sector 18: 128KB */
    0x08160000U, /* Sector 19: 128KB */
    0x08180000U, /* Sector 20: 128KB */
    0x081A0000U, /* Sector 21: 128KB */
    0x081C0000U, /* Sector 22: 128KB */
    0x081E0000U, /* Sector 23: 128KB */
    0x08200000U  /* End address */
};

/* Public functions ----------------------------------------------------------*/

uint32_t FLASH_GetSector(uint32_t address) {
    if (!FLASH_IsValidAddress(address)) {
        return FLASH_SECTOR_NOT_FOUND;
    }

    for (uint32_t i = 0; i < FLASH_SECTOR_TOTAL; i++) {
        if (address >= FLASH_SECTOR_ADDRESSES[i] && address < FLASH_SECTOR_ADDRESSES[i + 1]) {
            return i;
        }
    }

    return FLASH_SECTOR_NOT_FOUND;
}

FLASH_StatusTypeDef FLASH_GetSectorInfo(uint32_t sector, FLASH_SectorInfoTypeDef *info) {
    if (info == NULL || sector >= FLASH_SECTOR_TOTAL) {
        return FLASH_STATUS_INVALID_PARAM;
    }

    info->SectorNumber = sector;
    info->StartAddress = FLASH_SECTOR_ADDRESSES[sector];
    info->Size = FLASH_SECTOR_ADDRESSES[sector + 1] - FLASH_SECTOR_ADDRESSES[sector];

    return FLASH_STATUS_OK;
}

bool FLASH_IsValidAddress(uint32_t address) {
    return (address >= FLASH_BASE_ADDRESS && address <= FLASH_END_ADDRESS);
}

bool FLASH_IsValidRange(uint32_t address, uint32_t length) {
    if (length == 0U || !FLASH_IsValidAddress(address)) {
        return false;
    }

    /* Compare the remaining space instead of computing the last address: with
       a large enough length, address + length - 1 wraps back into Flash. */
    return (length <= (FLASH_END_ADDRESS - address) + 1U);
}
