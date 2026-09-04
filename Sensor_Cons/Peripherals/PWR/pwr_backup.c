/**
 ******************************************************************************
 * @file    pwr_backup.c
 * @brief   Backup domain access control
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "pwr_backup.h"

/* Public functions ----------------------------------------------------------*/

PWR_StatusTypeDef PWR_EnableBackupAccess(void) {
    HAL_PWR_EnableBkUpAccess();

    return PWR_OK;
}

PWR_StatusTypeDef PWR_DisableBackupAccess(void) {
    HAL_PWR_DisableBkUpAccess();

    return PWR_OK;
}

bool PWR_IsLSEReady(void) {
    return (RCC->BDCR & RCC_BDCR_LSERDY) != 0U;
}
