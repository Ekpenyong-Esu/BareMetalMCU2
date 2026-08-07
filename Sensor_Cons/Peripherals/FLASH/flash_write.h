/**
  ******************************************************************************
  * @file    flash_write.h
  * @brief   Flash programming operations
  ******************************************************************************
  */

#ifndef __FLASH_WRITE_H__
#define __FLASH_WRITE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "flash_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Write a byte to Flash
 * @param   address Flash address to write
 * @param   data Byte value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteByte(uint32_t address, uint8_t data);

/**
 * @brief   Write a half-word (16-bit) to Flash
 * @param   address Flash address to write (must be 2-byte aligned)
 * @param   data Half-word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteHalfWord(uint32_t address, uint16_t data);

/**
 * @brief   Write a word (32-bit) to Flash
 * @param   address Flash address to write (must be 4-byte aligned)
 * @param   data Word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteWord(uint32_t address, uint32_t data);

/**
 * @brief   Write a double-word (64-bit) to Flash
 * @param   address Flash address to write (must be 8-byte aligned)
 * @param   data Double-word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteDoubleWord(uint32_t address, uint64_t data);

/**
 * @brief   Write buffer to Flash
 * @details Programs an array of bytes and reads it back to confirm
 * @param   address Start address in Flash
 * @param   data Pointer to data buffer
 * @param   length Number of bytes to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t* data, uint32_t length);

/**
 * @brief   Write 32-bit buffer to Flash
 * @details Programs an array of words and reads it back to confirm
 * @param   address Start address in Flash (must be 4-byte aligned)
 * @param   data Pointer to data buffer
 * @param   count Number of 32-bit words to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteBuffer32(uint32_t address, const uint32_t* data, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_WRITE_H__ */
