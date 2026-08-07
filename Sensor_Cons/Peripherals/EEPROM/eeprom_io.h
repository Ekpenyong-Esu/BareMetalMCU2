/**
 * @file    eeprom_io.h
 * @brief   EEPROM transport layer (internal)
 * @details The only EEPROM module that knows about I2C. Everything above it
 *          works in terms of memory addresses and byte counts.
 * @note    Internal to the driver; not part of the eeprom.h aggregator.
 */

#ifndef __EEPROM_IO_H__
#define __EEPROM_IO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "eeprom_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Bring up the shared I2C bus if the application has not already
 * @retval  None
 */
void EEPROM_IO_Init(void);

/**
 * @brief   Write a byte range to the device
 * @details Splits the transfer at block boundaries on 1-byte-address parts
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Source buffer
 * @param   length Number of bytes
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_IO_Write(const EEPROM_HandleTypeDef* handle,
                                     uint16_t address,
                                     const uint8_t* data,
                                     uint16_t length);

/**
 * @brief   Read a byte range from the device
 * @details Splits the transfer at block boundaries on 1-byte-address parts
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Destination buffer
 * @param   length Number of bytes
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_IO_Read(const EEPROM_HandleTypeDef* handle,
                                    uint16_t address,
                                    uint8_t* data,
                                    uint16_t length);

/**
 * @brief   ACK-poll the device until it answers
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef EEPROM_OK when the device acknowledges
 */
EEPROM_StatusTypeDef EEPROM_IO_IsDeviceReady(const EEPROM_HandleTypeDef* handle);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_IO_H__ */
