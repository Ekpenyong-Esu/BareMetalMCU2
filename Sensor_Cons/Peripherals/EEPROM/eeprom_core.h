/**
 * @file    eeprom_core.h
 * @brief   EEPROM lifecycle, device geometry and readiness
 */

#ifndef __EEPROM_CORE_H__
#define __EEPROM_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "eeprom_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Initialize EEPROM with default M24LR64 configuration
 * @param   handle Pointer to EEPROM handle
 * @param   bus I2C bus the chip is wired to, already opened with I2C_BusInit
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Init(EEPROM_HandleTypeDef *handle, I2C_Bus_t *bus);

/**
 * @brief   Initialize EEPROM with a known device type
 * @param   handle Pointer to EEPROM handle
 * @param   bus I2C bus the chip is wired to, already opened with I2C_BusInit
 * @param   type EEPROM type from EEPROM_TypeDef
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_InitType(EEPROM_HandleTypeDef *handle, I2C_Bus_t *bus,
                                     EEPROM_TypeDef type);

/**
 * @brief   Initialize EEPROM with custom configuration
 * @param   handle Pointer to EEPROM handle
 * @param   bus I2C bus the chip is wired to, already opened with I2C_BusInit
 * @param   config Pointer to configuration structure
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_InitCustom(EEPROM_HandleTypeDef *handle, I2C_Bus_t *bus,
                                       const EEPROM_ConfigTypeDef *config);

/**
 * @brief   Deinitialize EEPROM
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_DeInit(EEPROM_HandleTypeDef *handle);

/**
 * @brief   Wait for the EEPROM to complete its internal write cycle
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WaitReady(EEPROM_HandleTypeDef *handle);

/**
 * @brief   Check if the EEPROM answers on the bus
 * @param   handle Pointer to EEPROM handle
 * @retval  true if ready, false otherwise
 */
bool EEPROM_IsReady(EEPROM_HandleTypeDef *handle);

/**
 * @brief   Get EEPROM total size
 * @param   handle Pointer to EEPROM handle
 * @retval  Total size in bytes, 0 if not initialized
 */
uint32_t EEPROM_GetSize(EEPROM_HandleTypeDef *handle);

/**
 * @brief   Get EEPROM page size
 * @param   handle Pointer to EEPROM handle
 * @retval  Page size in bytes, 0 if not initialized
 */
uint16_t EEPROM_GetPageSize(EEPROM_HandleTypeDef *handle);

/**
 * @brief   Validate a handle and an address range against the device geometry
 * @details Shared entry guard for every read/write/utility entry point
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   length Number of bytes
 * @retval  EEPROM_StatusTypeDef EEPROM_OK when the range is usable
 */
EEPROM_StatusTypeDef EEPROM_CheckRange(const EEPROM_HandleTypeDef *handle, uint16_t address,
                                       uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_CORE_H__ */
