/**
 ******************************************************************************
 * @file    flash_read.h
 * @brief   Flash read-back and blank-check operations
 ******************************************************************************
 */

#ifndef __FLASH_READ_H__
#define __FLASH_READ_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "flash_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Read a byte from Flash
 * @param   address Flash address to read
 * @retval  uint8_t Byte value at address
 */
uint8_t FLASH_ReadByte(uint32_t address);

/**
 * @brief   Read a half-word from Flash
 * @param   address Flash address to read (should be 2-byte aligned)
 * @retval  uint16_t Half-word value at address
 */
uint16_t FLASH_ReadHalfWord(uint32_t address);

/**
 * @brief   Read a word from Flash
 * @param   address Flash address to read (should be 4-byte aligned)
 * @retval  uint32_t Word value at address
 */
uint32_t FLASH_ReadWord(uint32_t address);

/**
 * @brief   Read buffer from Flash
 * @param   address Start address in Flash
 * @param   data Pointer to destination buffer
 * @param   length Number of bytes to read
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_ReadBuffer(uint32_t address, uint8_t *data, uint32_t length);

/**
 * @brief   Check if Flash region is erased
 * @details Verifies all bytes in region are 0xFF
 * @param   address Start address
 * @param   length Number of bytes to check
 * @retval  bool True if region is erased
 */
bool FLASH_IsErased(uint32_t address, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_READ_H__ */
