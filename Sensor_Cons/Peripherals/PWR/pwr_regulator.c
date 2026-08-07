/**
  ******************************************************************************
  * @file    pwr_regulator.c
  * @brief   Internal voltage regulator scaling
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwr_regulator.h"

/* Public functions ----------------------------------------------------------*/

PWR_StatusTypeDef PWR_EnableHighPerformance(void)
{
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    return PWR_OK;
}

PWR_StatusTypeDef PWR_EnableLowPowerMode(void)
{
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    return PWR_OK;
}
