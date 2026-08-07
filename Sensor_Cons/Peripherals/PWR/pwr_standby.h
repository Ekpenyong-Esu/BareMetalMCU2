/**
  ******************************************************************************
  * @file    pwr_standby.h
  * @brief   Standby mode entry, WKUP pin control and standby wakeup detection
  ******************************************************************************
  */

#ifndef __PWR_STANDBY_H__
#define __PWR_STANDBY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Enter Standby mode
 * @details Lowest power mode, only wakeup via WKUP pin, RTC, or reset.
 *          All RAM content is lost except the backup domain.
 * @retval  None (does not return; the system resets on wakeup)
 */
void PWR_EnterStandbyMode(void);

/**
 * @brief   Enable wakeup pin for Standby mode
 * @details PA0 (WKUP pin) can wake from Standby
 * @param   enable True to enable, false to disable
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableWakeupPin(bool enable);

/**
 * @brief   Check if the last reset was a Standby wakeup
 * @details Call early in main() to check the wakeup source
 * @retval  bool True if wakeup was from Standby mode
 */
bool PWR_WasStandbyWakeup(void);

/**
 * @brief   Clear the Standby flag
 * @details Should be called after checking the Standby wakeup
 * @retval  None
 */
void PWR_ClearStandbyFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_STANDBY_H__ */
