/**
 * @file    eeprom_write.h
 * @brief   EEPROM write operations
 */

#ifndef __EEPROM_WRITE_H__
#define __EEPROM_WRITE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "eeprom_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Write a single byte and wait for the write cycle
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Byte to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteByte(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t data);

/**
 * @brief   Write a byte range, splitting it across pages as needed
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Source buffer
 * @param   length Number of bytes to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Write(EEPROM_HandleTypeDef *handle, uint16_t address,
                                  const uint8_t *data, uint16_t length);

/**
 * @brief   Write within a single page and wait for the write cycle
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting address
 * @param   data Source buffer
 * @param   length Number of bytes; the range must not cross a page boundary
 * @retval  EEPROM_StatusTypeDef EEPROM_INVALID_PARAM if the range would cross
 *          a page boundary, since the device wraps instead of advancing
 */
EEPROM_StatusTypeDef EEPROM_WritePage(EEPROM_HandleTypeDef *handle, uint16_t address,
                                      const uint8_t *data, uint8_t length);

/**
 * @brief   Write a 16-bit value big-endian
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Value to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                      uint16_t data);

/**
 * @brief   Write a 32-bit value big-endian
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Value to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteDWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                       uint32_t data);

/**
 * @brief   Write a float in native byte order
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Value to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteFloat(EEPROM_HandleTypeDef *handle, uint16_t address, float data);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_WRITE_H__ */
