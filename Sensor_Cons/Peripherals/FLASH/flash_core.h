/**
 ******************************************************************************
 * @file    flash_core.h
 * @brief   Flash access control: lock state, completion and status reporting
 ******************************************************************************
 */

#ifndef __FLASH_CORE_H__
#define __FLASH_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "flash_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Unlock Flash for programming
 * @details Must be called before any write/erase operation
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_Unlock(void);

/**
 * @brief   Lock Flash to prevent accidental writes
 * @details Should be called after write/erase operations
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_Lock(void);

/**
 * @brief   Wait for Flash operation to complete
 * @details Polls busy flag until operation completes
 * @param   timeout Timeout in milliseconds
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WaitForOperation(uint32_t timeout);

/**
 * @brief   Get Flash status string
 * @details Converts status code to human-readable string
 * @param   status Flash status code
 * @retval  const char* Status description string
 */
const char *FLASH_GetStatusString(FLASH_StatusTypeDef status);

/**
 * @brief   Convert HAL status to FLASH status
 * @note    Internal to the driver, shared by the erase and write modules.
 * @param   halStatus HAL status code
 * @retval  FLASH_StatusTypeDef Converted status
 */
FLASH_StatusTypeDef FLASH_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/**
 * @brief   Clear every pending Flash error flag
 * @note    Internal to the driver. A stale flag makes the next
 *          FLASH_WaitForLastOperation() report a failure that already happened.
 * @param   None
 * @retval  None
 */
void FLASH_ClearErrorFlags(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_CORE_H__ */
