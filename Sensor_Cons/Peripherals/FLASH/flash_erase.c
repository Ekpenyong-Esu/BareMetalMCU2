/**
  ******************************************************************************
  * @file    flash_erase.c
  * @brief   Flash sector erase operations
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "flash_erase.h"
#include "flash_core.h"

/* Private defines -----------------------------------------------------------*/

/** @brief HAL_FLASHEx_Erase reports this sector index when every sector succeeded */
#define FLASH_ERASE_ALL_OK      0xFFFFFFFFU

/* Public functions ----------------------------------------------------------*/

FLASH_StatusTypeDef FLASH_EraseSector(uint32_t sector)
{
    return FLASH_EraseSectors(sector, sector);
}

FLASH_StatusTypeDef FLASH_EraseSectors(uint32_t startSector, uint32_t endSector)
{
    /* Zero-initialised: HAL reads fields this driver does not set, notably
       Banks, and an uninitialised struct would hand it stack garbage. */
    FLASH_EraseInitTypeDef eraseInit = {0};
    uint32_t sectorError = 0;
    FLASH_StatusTypeDef status;
    HAL_StatusTypeDef halStatus;

    if (startSector > endSector || endSector >= FLASH_SECTOR_TOTAL)
    {
        return FLASH_STATUS_INVALID_PARAM;
    }

    status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    FLASH_ClearErrorFlags();

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;  /* 2.7V to 3.6V */
    eraseInit.Banks = (startSector < FLASH_SECTOR_BANK1) ? FLASH_BANK_1 : FLASH_BANK_2;
    eraseInit.Sector = startSector;
    eraseInit.NbSectors = endSector - startSector + 1;

    halStatus = HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    (void)FLASH_Lock();

    if (halStatus != HAL_OK || sectorError != FLASH_ERASE_ALL_OK)
    {
        return FLASH_STATUS_ERROR;
    }

    return FLASH_STATUS_OK;
}

FLASH_StatusTypeDef FLASH_EraseUserSector(void)
{
    return FLASH_EraseSector(FLASH_USER_SECTOR);
}
