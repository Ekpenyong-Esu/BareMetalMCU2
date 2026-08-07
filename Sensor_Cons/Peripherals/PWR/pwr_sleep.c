/**
  ******************************************************************************
  * @file    pwr_sleep.c
  * @brief   Sleep mode entry
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwr_sleep.h"

/* Public functions ----------------------------------------------------------*/

void PWR_EnterSleepMode(PWR_SleepModeTypeDef mode)
{
    if (mode == PWR_SLEEP_MODE_WFI)
    {
        /* Ensure outstanding memory transactions complete before halting */
        __DSB();
        __ISB();

        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    }
    else
    {
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFE);
    }
}

void PWR_SleepFor(uint32_t duration_ms)
{
    uint32_t startTick = HAL_GetTick();

    while ((HAL_GetTick() - startTick) < duration_ms)
    {
        PWR_EnterSleepMode(PWR_SLEEP_MODE_WFI);
    }
}
