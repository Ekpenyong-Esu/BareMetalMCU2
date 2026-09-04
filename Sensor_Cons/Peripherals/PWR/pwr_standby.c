/**
 ******************************************************************************
 * @file    pwr_standby.c
 * @brief   Standby mode entry and wakeup detection
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "pwr_standby.h"

/* Public functions ----------------------------------------------------------*/

void PWR_EnterStandbyMode(void) {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

    HAL_PWR_EnterSTANDBYMode();

    /* Never reached - the system resets on wakeup */
}

PWR_StatusTypeDef PWR_EnableWakeupPin(bool enable) {
    if (enable) {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    }
    else {
        HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    }

    return PWR_OK;
}

bool PWR_WasStandbyWakeup(void) {
    return __HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET;
}

void PWR_ClearStandbyFlag(void) {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
}
