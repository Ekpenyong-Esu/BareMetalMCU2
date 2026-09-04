/**
 * @file    eeprom_read.h
 * @brief   EEPROM read operations
 */

#ifndef __EEPROM_READ_H__
#define __EEPROM_READ_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "eeprom_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Read a single byte
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store the read byte
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadByte(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t *data);

/**
 * @brief   Read a byte range
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Destination buffer
 * @param   length Number of bytes to read
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Read(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t *data,
                                 uint16_t length);

/**
 * @brief   Read a 16-bit big-endian value
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store the value
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                     uint16_t *data);

/**
 * @brief   Read a 32-bit big-endian value
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store the value
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadDWord(EEPROM_HandleTypeDef *handle, uint16_t address,
                                      uint32_t *data);

/**
 * @brief   Read a float stored in native byte order
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store the value
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadFloat(EEPROM_HandleTypeDef *handle, uint16_t address, float *data);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_READ_H__ */
