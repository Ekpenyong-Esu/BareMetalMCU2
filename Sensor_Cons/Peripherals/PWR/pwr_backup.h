/**
 ******************************************************************************
 * @file    pwr_backup.h
 * @brief   Backup domain access control
 ******************************************************************************
 */

#ifndef __PWR_BACKUP_H__
#define __PWR_BACKUP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Enable write access to the backup domain
 * @details Required before accessing the RTC and the backup registers
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableBackupAccess(void);

/**
 * @brief   Disable write access to the backup domain
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_DisableBackupAccess(void);

/**
 * @brief   Check whether the backup domain LSE oscillator is running
 * @note    The STM32F4 has no VBAT-present status bit. LSE readiness is the
 *          only observable sign that the backup domain is powered and clocked.
 * @retval  bool True if LSE is ready
 */
bool PWR_IsLSEReady(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_BACKUP_H__ */
