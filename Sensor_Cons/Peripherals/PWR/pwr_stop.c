/**
  ******************************************************************************
  * @file    pwr_stop.c
  * @brief   Stop mode entry and post-wakeup clock restoration
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwr_stop.h"
#include "sys.h"
#include "log.h"

/* Public functions ----------------------------------------------------------*/

void PWR_EnterStopMode(PWR_RegulatorTypeDef regulator, PWR_StopEntryTypeDef entry)
{
    const uint32_t regulatorMode = (regulator == PWR_REGULATOR_LOW_POWER) ?
                                   PWR_LOWPOWERREGULATOR_ON : PWR_MAINREGULATOR_ON;
    const uint32_t entryMode = (entry == PWR_STOP_ENTRY_WFE) ?
                               PWR_STOPENTRY_WFE : PWR_STOPENTRY_WFI;

    HAL_PWR_EnterSTOPMode(regulatorMode, entryMode);

    /* Execution resumes here after wakeup, running from HSI */
}

PWR_StatusTypeDef PWR_ConfigureAfterStop(void)
{
    /* Delegating keeps a single source of truth for the clock tree: a private
       copy here silently ran the core at a different frequency than at boot. */
    SystemClock_Config();

    log_debug("PWR: System clocks restored after Stop mode");

    return PWR_OK;
}
