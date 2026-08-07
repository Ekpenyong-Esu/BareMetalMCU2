/**
  ******************************************************************************
  * @file    pwr_low_power.h
  * @brief   Low power orchestration built on the sleep/stop/standby modules
  ******************************************************************************
  */

#ifndef __PWR_LOW_POWER_H__
#define __PWR_LOW_POWER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Enter low power mode with configuration
 * @details Unified entry point that dispatches to the light/deep/standby paths
 * @param   config Pointer to low power configuration
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnterLowPowerMode(const PWR_LowPowerConfigTypeDef* config);

/**
 * @brief   Enter light low power mode (Sleep)
 * @param   wakeupSources Wakeup sources to configure
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnterLightLowPower(PWR_WakeupSourceTypeDef wakeupSources, bool keepPeripherals);

/**
 * @brief   Enter deep low power mode (Stop)
 * @param   wakeupSources Wakeup sources to configure
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnterDeepLowPower(PWR_WakeupSourceTypeDef wakeupSources, bool keepPeripherals);

/**
 * @brief   Enter standby low power mode
 * @param   wakeupSources Wakeup sources to configure
 * @retval  PWR_StatusTypeDef Operation status (only returns on failure)
 */
PWR_StatusTypeDef PWR_EnterStandbyLowPower(PWR_WakeupSourceTypeDef wakeupSources);

/**
 * @brief   Auto-select and enter the optimal low power mode
 * @details Sleep is chosen for very short waits or when peripherals must stay
 *          alive, Stop otherwise. Standby is never selected automatically
 *          because it loses RAM; request it explicitly.
 * @param   wakeupTimeMs Expected time until wakeup (milliseconds)
 * @param   keepPeripherals Keep critical peripherals active
 * @param   wakeupSources Wakeup sources to configure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_AutoLowPowerMode(uint32_t wakeupTimeMs, bool keepPeripherals, PWR_WakeupSourceTypeDef wakeupSources);

/**
 * @brief   Configure wakeup sources for low power modes
 * @details Enables the WKUP pin and unmasks the RTC EXTI lines so the
 *          corresponding events can wake the core from Stop mode
 * @param   sources Wakeup sources to enable
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureWakeupSources(PWR_WakeupSourceTypeDef sources);

/**
 * @brief   Get default low power configuration
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetDefaultLowPowerConfig(PWR_LowPowerConfigTypeDef* config);

/**
 * @brief   Optimize system for low power consumption
 * @details Disables unnecessary clocks and peripherals
 * @note    Weak: applications override this with their own strategy
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals);

/**
 * @brief   Restore system after low power wakeup
 * @note    Weak: applications override this with their own restore sequence
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_RestoreFromLowPower(void);

/**
 * @brief   Report the last low power mode this driver entered
 * @details After a Standby wakeup the core is reset, so the recorded mode is
 *          lost. In that case the standby flag is used instead and the wakeup
 *          source is reported as PWR_SRC_WAKEUP_PIN. The standby flag is left
 *          set; call PWR_ClearStandbyFlag() when done with it.
 * @param   mode Pointer to store the mode (required)
 * @param   wakeupSource Pointer to store the configured sources (optional)
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetLowPowerStatus(PWR_LowPowerModeTypeDef* mode, PWR_WakeupSourceTypeDef* wakeupSource);

/**
 * @brief   Configure advanced low power settings
 * @param   flashPowerDown Power the flash down while in Stop mode
 * @param   disableBackupWrites Disable backup register writes to save power
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureAdvancedLowPower(bool flashPowerDown, bool disableBackupWrites);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_LOW_POWER_H__ */
