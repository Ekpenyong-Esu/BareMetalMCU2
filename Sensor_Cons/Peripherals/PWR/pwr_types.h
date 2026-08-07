/**
  ******************************************************************************
  * @file    pwr_types.h
  * @brief   Power management vocabulary: status codes, modes and configuration
  * @details Pure data definitions shared by every PWR module. Contains no
  *          behaviour, so module headers can include it without cycles.
  ******************************************************************************
  */

#ifndef __PWR_TYPES_H__
#define __PWR_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief PWR Status enumeration
 */
typedef enum {
    PWR_OK = 0,                /**< Operation completed successfully */
    PWR_ERROR,                 /**< General error occurred */
    PWR_INVALID_PARAM,         /**< Invalid parameter provided */
    PWR_TIMEOUT,               /**< Operation timed out */
    PWR_NOT_READY              /**< Power system not ready */
} PWR_StatusTypeDef;

/**
 * @brief Sleep mode types
 */
typedef enum {
    PWR_SLEEP_MODE_WFI = 0,    /**< Wait For Interrupt - CPU sleeps */
    PWR_SLEEP_MODE_WFE         /**< Wait For Event - CPU sleeps */
} PWR_SleepModeTypeDef;

/**
 * @brief Stop mode entry types
 */
typedef enum {
    PWR_STOP_ENTRY_WFI = 0,    /**< Enter Stop with WFI instruction */
    PWR_STOP_ENTRY_WFE         /**< Enter Stop with WFE instruction */
} PWR_StopEntryTypeDef;

/**
 * @brief Voltage regulator mode for Stop mode
 */
typedef enum {
    PWR_REGULATOR_ON = 0,      /**< Main regulator ON during Stop */
    PWR_REGULATOR_LOW_POWER    /**< Low-power regulator during Stop */
} PWR_RegulatorTypeDef;

/**
 * @brief Standby mode wakeup source
 * @note  Uses PWR_SRC_ prefix to avoid conflicts with HAL PWR_WAKEUP_PIN macros
 * @note  For HAL functions, use HAL's PWR_WAKEUP_PIN1 macro directly
 */
typedef enum {
    PWR_SRC_NONE = 0x000,              /**< No wakeup source configured */
    PWR_SRC_WAKEUP_PIN = 0x001,        /**< WKUP pin (PA0) - use PWR_WAKEUP_PIN1 for HAL */
    PWR_SRC_RTC_ALARM = 0x002,         /**< RTC Alarm wakeup */
    PWR_SRC_RTC_WAKEUP = 0x004,        /**< RTC Wakeup timer */
    PWR_SRC_RTC_TIMESTAMP = 0x008      /**< RTC Timestamp */
} PWR_WakeupSourceTypeDef;

/**
 * @brief PVD (Programmable Voltage Detector) threshold
 */
typedef enum {
    PWR_PVD_LEVEL_2V0 = PWR_CR_PLS_LEV0, /**< 2.0V threshold */
    PWR_PVD_LEVEL_2V1 = PWR_CR_PLS_LEV1, /**< 2.1V threshold */
    PWR_PVD_LEVEL_2V3 = PWR_CR_PLS_LEV2, /**< 2.3V threshold */
    PWR_PVD_LEVEL_2V5 = PWR_CR_PLS_LEV3, /**< 2.5V threshold */
    PWR_PVD_LEVEL_2V6 = PWR_CR_PLS_LEV4, /**< 2.6V threshold */
    PWR_PVD_LEVEL_2V7 = PWR_CR_PLS_LEV5, /**< 2.7V threshold */
    PWR_PVD_LEVEL_2V8 = PWR_CR_PLS_LEV6, /**< 2.8V threshold */
    PWR_PVD_LEVEL_2V9 = PWR_CR_PLS_LEV7  /**< 2.9V threshold */
} PWR_PVDLevelTypeDef;

/**
 * @brief Power configuration structure
 */
typedef struct {
    bool enablePVD;                     /**< Enable Programmable Voltage Detector */
    PWR_PVDLevelTypeDef pvdLevel;       /**< PVD threshold level */
    bool enableBackupAccess;            /**< Enable backup domain access */
    bool enableWakeupPin;               /**< Enable WKUP pin */
} PWR_ConfigTypeDef;

/**
 * @brief Low power mode levels
 */
typedef enum {
    PWR_LOW_POWER_MODE_LIGHT = 0,      /**< Light sleep - CPU sleep, peripherals active */
    PWR_LOW_POWER_MODE_DEEP,            /**< Deep sleep - Stop mode with low power regulator */
    PWR_LOW_POWER_MODE_STANDBY,         /**< Standby - Lowest power, RAM lost */
    PWR_LOW_POWER_MODE_AUTO             /**< Auto-select based on wakeup time and peripherals */
} PWR_LowPowerModeTypeDef;

/**
 * @brief Low power configuration structure
 */
typedef struct {
    PWR_LowPowerModeTypeDef mode;       /**< Low power mode to enter */
    uint32_t wakeupTimeMs;              /**< Expected wakeup time (for auto mode) */
    bool keepPeripherals;               /**< Keep critical peripherals active */
    PWR_WakeupSourceTypeDef wakeupSources; /**< Wakeup sources to enable */
    bool optimizeVoltage;               /**< Optimize voltage regulator for low power */
} PWR_LowPowerConfigTypeDef;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief Backup registers count (STM32F429 has 20 backup registers)
 */
#define PWR_BACKUP_REG_COUNT    20U

/**
 * @brief Default configuration timeout
 */
#define PWR_TIMEOUT_VALUE       1000U   /**< 1 second */

#ifdef __cplusplus
}
#endif

#endif /* __PWR_TYPES_H__ */
