/**
 ******************************************************************************
 * @file    pwr_stop.h
 * @brief   Stop mode entry and post-wakeup clock restoration
 ******************************************************************************
 */

#ifndef __PWR_STOP_H__
#define __PWR_STOP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Enter Stop mode
 * @details Most clocks stopped, RAM retained, wake via interrupt or event
 * @param   regulator Regulator mode (ON or LOW_POWER)
 * @param   entry Entry mode (WFI or WFE)
 * @retval  None
 */
void PWR_EnterStopMode(PWR_RegulatorTypeDef regulator, PWR_StopEntryTypeDef entry);

/**
 * @brief   Restore the system clock tree after a Stop mode wakeup
 * @details Stop mode leaves the system running from HSI at 16 MHz. This
 *          re-runs the board clock configuration so every derived rate
 *          (SysTick, APB prescalers, baud rates) matches the boot values.
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureAfterStop(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_STOP_H__ */
