/**
 ******************************************************************************
 * @file    pwr.c
 * @brief   PWR initialisation and configuration
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "pwr_core.h"
#include "pwr_pvd.h"
#include "pwr_standby.h"
#include "pwr_backup.h"
#include "log.h"

/* Public functions ----------------------------------------------------------*/

PWR_StatusTypeDef PWR_Init(const PWR_ConfigTypeDef *config) {
    if (config == NULL) {
        return PWR_INVALID_PARAM;
    }

    __HAL_RCC_PWR_CLK_ENABLE();

    if (config->enableBackupAccess) {
        PWR_EnableBackupAccess();
    }

    if (config->enableWakeupPin) {
        PWR_EnableWakeupPin(true);
    }

    if (config->enablePVD) {
        PWR_EnablePVD(config->pvdLevel);
    }

    log_debug("PWR: Power Management initialized");

    return PWR_OK;
}

PWR_StatusTypeDef PWR_InitDefault(void) {
    PWR_ConfigTypeDef config;

    PWR_GetDefaultConfig(&config);

    return PWR_Init(&config);
}

PWR_StatusTypeDef PWR_GetDefaultConfig(PWR_ConfigTypeDef *config) {
    if (config == NULL) {
        return PWR_INVALID_PARAM;
    }

    config->enablePVD = false;
    config->pvdLevel = PWR_PVD_LEVEL_2V9;
    config->enableBackupAccess = true;
    config->enableWakeupPin = false;

    return PWR_OK;
}
