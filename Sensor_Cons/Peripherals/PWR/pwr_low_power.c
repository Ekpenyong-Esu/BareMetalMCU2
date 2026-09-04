/**
 ******************************************************************************
 * @file    pwr_low_power.c
 * @brief   Low power orchestration built on the sleep/stop/standby modules
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "pwr_low_power.h"
#include "pwr_sleep.h"
#include "pwr_stop.h"
#include "pwr_standby.h"
#include "pwr_regulator.h"
#include "pwr_backup.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/
/** Below this expected wait, Stop mode entry/exit costs more than it saves */
#define PWR_AUTO_SLEEP_THRESHOLD_MS 10U
/** Wake-up interval handed out by PWR_GetDefaultLowPowerConfig() */
#define PWR_DEFAULT_WAKEUP_TIME_MS 1000U

/* Private variables ---------------------------------------------------------*/
static PWR_LowPowerModeTypeDef s_lastMode = PWR_LOW_POWER_MODE_LIGHT;
static PWR_WakeupSourceTypeDef s_lastSources = PWR_SRC_NONE;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Common preparation shared by every low power entry path
 */
static PWR_StatusTypeDef PWR_PrepareLowPower(PWR_LowPowerModeTypeDef mode,
                                             PWR_WakeupSourceTypeDef wakeupSources,
                                             bool keepPeripherals) {
    PWR_StatusTypeDef status = PWR_ConfigureWakeupSources(wakeupSources);

    if (status != PWR_OK) {
        log_error("PWR: Failed to configure wakeup sources");
        return status;
    }

    PWR_OptimizeForLowPower(keepPeripherals);

    s_lastMode = mode;
    s_lastSources = wakeupSources;

    return PWR_OK;
}

/* Public functions ----------------------------------------------------------*/

PWR_StatusTypeDef PWR_GetDefaultLowPowerConfig(PWR_LowPowerConfigTypeDef *config) {
    if (config == NULL) {
        return PWR_INVALID_PARAM;
    }

    config->mode = PWR_LOW_POWER_MODE_AUTO;
    config->wakeupTimeMs = PWR_DEFAULT_WAKEUP_TIME_MS;
    config->keepPeripherals = false;
    config->wakeupSources = PWR_SRC_WAKEUP_PIN;
    config->optimizeVoltage = true;

    return PWR_OK;
}

PWR_StatusTypeDef PWR_EnterLowPowerMode(const PWR_LowPowerConfigTypeDef *config) {
    PWR_StatusTypeDef status = PWR_OK;

    if (config == NULL) {
        return PWR_INVALID_PARAM;
    }

    log_debug("PWR: Entering low power mode %d", config->mode);

    if (config->optimizeVoltage) {
        PWR_EnableLowPowerMode();
    }

    switch (config->mode) {
        case PWR_LOW_POWER_MODE_LIGHT:
            status = PWR_EnterLightLowPower(config->wakeupSources, config->keepPeripherals);
            break;

        case PWR_LOW_POWER_MODE_DEEP:
            status = PWR_EnterDeepLowPower(config->wakeupSources, config->keepPeripherals);
            break;

        case PWR_LOW_POWER_MODE_STANDBY:
            status = PWR_EnterStandbyLowPower(config->wakeupSources);
            break;

        case PWR_LOW_POWER_MODE_AUTO:
            status = PWR_AutoLowPowerMode(config->wakeupTimeMs, config->keepPeripherals,
                                          config->wakeupSources);
            break;

        default:
            status = PWR_INVALID_PARAM;
            break;
    }

    return status;
}

PWR_StatusTypeDef PWR_EnterLightLowPower(PWR_WakeupSourceTypeDef wakeupSources,
                                         bool keepPeripherals) {
    PWR_StatusTypeDef status =
        PWR_PrepareLowPower(PWR_LOW_POWER_MODE_LIGHT, wakeupSources, keepPeripherals);

    if (status != PWR_OK) {
        return status;
    }

    /* Suspend SysTick so its 1 kHz interrupt does not wake us immediately */
    HAL_SuspendTick();

    PWR_EnterSleepMode(PWR_SLEEP_MODE_WFI);

    HAL_ResumeTick();

    log_debug("PWR: Exited light low power mode");

    return PWR_OK;
}

PWR_StatusTypeDef PWR_EnterDeepLowPower(PWR_WakeupSourceTypeDef wakeupSources,
                                        bool keepPeripherals) {
    PWR_StatusTypeDef status =
        PWR_PrepareLowPower(PWR_LOW_POWER_MODE_DEEP, wakeupSources, keepPeripherals);

    if (status != PWR_OK) {
        return status;
    }

    HAL_SuspendTick();

    /* The WKUP pin wakes through an interrupt; EXTI events are cheaper via WFE */
    PWR_StopEntryTypeDef stopEntry =
        (wakeupSources & PWR_SRC_WAKEUP_PIN) ? PWR_STOP_ENTRY_WFI : PWR_STOP_ENTRY_WFE;

    PWR_EnterStopMode(PWR_REGULATOR_LOW_POWER, stopEntry);

    HAL_ResumeTick();

    status = PWR_ConfigureAfterStop();
    if (status != PWR_OK) {
        log_error("PWR: Failed to restore after Stop mode");
        return status;
    }

    PWR_RestoreFromLowPower();

    log_debug("PWR: Exited deep low power mode");

    return PWR_OK;
}

PWR_StatusTypeDef PWR_EnterStandbyLowPower(PWR_WakeupSourceTypeDef wakeupSources) {
    PWR_StatusTypeDef status =
        PWR_PrepareLowPower(PWR_LOW_POWER_MODE_STANDBY, wakeupSources, false);

    if (status != PWR_OK) {
        return status;
    }

    PWR_EnterStandbyMode();

    /* Only reached if the core refused to enter Standby */
    return PWR_ERROR;
}

PWR_StatusTypeDef PWR_AutoLowPowerMode(uint32_t wakeupTimeMs, bool keepPeripherals,
                                       PWR_WakeupSourceTypeDef wakeupSources) {
    if (wakeupTimeMs < PWR_AUTO_SLEEP_THRESHOLD_MS || keepPeripherals) {
        log_debug("PWR: Auto-selected Sleep for %lu ms wakeup", wakeupTimeMs);
        return PWR_EnterLightLowPower(wakeupSources, keepPeripherals);
    }

    log_debug("PWR: Auto-selected Stop for %lu ms wakeup", wakeupTimeMs);

    return PWR_EnterDeepLowPower(wakeupSources, keepPeripherals);
}

PWR_StatusTypeDef PWR_ConfigureWakeupSources(PWR_WakeupSourceTypeDef sources) {
    if (sources & PWR_SRC_WAKEUP_PIN) {
        PWR_StatusTypeDef status = PWR_EnableWakeupPin(true);

        if (status != PWR_OK) {
            return status;
        }
    }

    /* Stop mode only halts the core: the RTC event still has to reach it
       through EXTI, which is masked by default. The RTC driver owns the
       alarm/timer values; PWR owns the wakeup path. */
    if (sources & PWR_SRC_RTC_ALARM) {
        __HAL_RTC_ALARM_EXTI_ENABLE_IT();
        __HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();
    }

    if (sources & PWR_SRC_RTC_WAKEUP) {
        __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_IT();
        __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_RISING_EDGE();
    }

    if (sources & PWR_SRC_RTC_TIMESTAMP) {
        __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_IT();
        __HAL_RTC_TAMPER_TIMESTAMP_EXTI_ENABLE_RISING_EDGE();
    }

    return PWR_OK;
}

__weak PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals) {
    log_debug("PWR: Optimizing system for low power (keep peripherals: %d)", keepPeripherals);

    return PWR_OK;
}

__weak PWR_StatusTypeDef PWR_RestoreFromLowPower(void) {
    log_debug("PWR: Restoring system after low power wakeup");

    return PWR_OK;
}

PWR_StatusTypeDef PWR_GetLowPowerStatus(PWR_LowPowerModeTypeDef *mode,
                                        PWR_WakeupSourceTypeDef *wakeupSource) {
    if (mode == NULL) {
        return PWR_INVALID_PARAM;
    }

    if (PWR_WasStandbyWakeup()) {
        /* Standby resets the core, so the statics above are back at defaults */
        *mode = PWR_LOW_POWER_MODE_STANDBY;

        if (wakeupSource != NULL) {
            *wakeupSource = PWR_SRC_WAKEUP_PIN;
        }
    }
    else {
        *mode = s_lastMode;

        if (wakeupSource != NULL) {
            *wakeupSource = s_lastSources;
        }
    }

    return PWR_OK;
}

PWR_StatusTypeDef PWR_ConfigureAdvancedLowPower(bool flashPowerDown, bool disableBackupWrites) {
    if (flashPowerDown) {
        HAL_PWREx_EnableFlashPowerDown();
    }
    else {
        HAL_PWREx_DisableFlashPowerDown();
    }

    if (disableBackupWrites) {
        PWR_DisableBackupAccess();
    }
    else {
        PWR_EnableBackupAccess();
    }

    return PWR_OK;
}
