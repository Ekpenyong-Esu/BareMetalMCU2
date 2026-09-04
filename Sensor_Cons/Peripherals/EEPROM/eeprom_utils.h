/**
 * @file    eeprom_utils.h
 * @brief   EEPROM bulk erase, verification and self test
 */

#ifndef __EEPROM_UTILS_H__
#define __EEPROM_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "eeprom_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Erase the whole device (write 0xFF everywhere)
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef Operation status
 * @note    This operation can take several seconds
 */
EEPROM_StatusTypeDef EEPROM_Erase(EEPROM_HandleTypeDef *handle);

/**
 * @brief   Erase a range of EEPROM memory
 * @param   handle Pointer to EEPROM handle
 * @param   startAddress Starting address
 * @param   length Number of bytes to erase
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_EraseRange(EEPROM_HandleTypeDef *handle, uint16_t startAddress,
                                       uint16_t length);

/**
 * @brief   Verify that stored data matches a buffer
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Buffer to compare against
 * @param   length Number of bytes to verify
 * @retval  EEPROM_StatusTypeDef EEPROM_OK if match, EEPROM_ERROR if mismatch
 */
EEPROM_StatusTypeDef EEPROM_Verify(EEPROM_HandleTypeDef *handle, uint16_t address,
                                   const uint8_t *data, uint16_t length);

/**
 * @brief   Write a test pattern, read it back and restore the original bytes
 * @param   handle Pointer to EEPROM handle
 * @param   testAddress Address to use for the test
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Test(EEPROM_HandleTypeDef *handle, uint16_t testAddress);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_UTILS_H__ */
