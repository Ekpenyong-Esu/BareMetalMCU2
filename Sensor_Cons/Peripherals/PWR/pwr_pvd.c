/**
 ******************************************************************************
 * @file    pwr_pvd.c
 * @brief   Programmable Voltage Detector control
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "pwr_pvd.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/
#define PWR_PVD_IRQ_PRIORITY 0U

/* Public functions ----------------------------------------------------------*/

PWR_StatusTypeDef PWR_EnablePVD(PWR_PVDLevelTypeDef level) {
    PWR_PVDTypeDef pvdConfig;

    pvdConfig.PVDLevel = level;
    pvdConfig.Mode = PWR_PVD_MODE_IT_RISING_FALLING;

    HAL_PWR_ConfigPVD(&pvdConfig);
    HAL_PWR_EnablePVD();

    return PWR_OK;
}

PWR_StatusTypeDef PWR_DisablePVD(void) {
    HAL_PWR_DisablePVD();

    return PWR_OK;
}

bool PWR_GetPVDStatus(void) {
    return __HAL_PWR_GET_FLAG(PWR_FLAG_PVDO) != RESET;
}

PWR_StatusTypeDef PWR_EnablePVDInterrupt(void) {
    HAL_NVIC_SetPriority(PVD_IRQn, PWR_PVD_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(PVD_IRQn);

    log_debug("PWR: PVD interrupt enabled");

    return PWR_OK;
}
