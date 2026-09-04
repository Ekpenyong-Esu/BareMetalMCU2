/**
 ******************************************************************************
 * @file    flash_sector.h
 * @brief   Flash sector geometry and address range validation
 ******************************************************************************
 */

#ifndef __FLASH_SECTOR_H__
#define __FLASH_SECTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "flash_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Get sector number from address
 * @details Returns sector number containing specified address
 * @param   address Flash address
 * @retval  uint32_t Sector number (0-23) or FLASH_SECTOR_NOT_FOUND if invalid
 */
uint32_t FLASH_GetSector(uint32_t address);

/**
 * @brief   Get sector information
 * @details Returns start address and size of specified sector
 * @param   sector Sector number
 * @param   info Pointer to sector info structure
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_GetSectorInfo(uint32_t sector, FLASH_SectorInfoTypeDef *info);

/**
 * @brief   Check if address is valid Flash address
 * @details Validates address is within Flash memory range
 * @param   address Address to validate
 * @retval  bool True if address is valid
 */
bool FLASH_IsValidAddress(uint32_t address);

/**
 * @brief   Check that a whole byte range lies inside Flash
 * @details Computes the range without ever forming address + length, so a
 *          length large enough to wrap uint32_t cannot slip through.
 * @param   address Start address
 * @param   length Number of bytes, must be non-zero
 * @retval  bool True if the entire range is inside Flash
 */
bool FLASH_IsValidRange(uint32_t address, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_SECTOR_H__ */
