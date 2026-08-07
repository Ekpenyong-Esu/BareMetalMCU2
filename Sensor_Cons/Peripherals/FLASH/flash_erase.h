/**
  ******************************************************************************
  * @file    flash_erase.h
  * @brief   Flash sector erase operations
  ******************************************************************************
  */

#ifndef __FLASH_ERASE_H__
#define __FLASH_ERASE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "flash_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Erase a Flash sector
 * @details Erases specified sector (sets all bytes to 0xFF)
 * @param   sector Sector number (0 to FLASH_SECTOR_TOTAL - 1)
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseSector(uint32_t sector);

/**
 * @brief   Erase multiple sectors
 * @details Erases sectors from startSector to endSector inclusive
 * @param   startSector First sector to erase
 * @param   endSector Last sector to erase
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseSectors(uint32_t startSector, uint32_t endSector);

/**
 * @brief   Erase user data sector
 * @details Erases the designated user data storage area
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseUserSector(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_ERASE_H__ */
