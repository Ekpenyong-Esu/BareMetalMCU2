/**
  ******************************************************************************
  * @file    pwr_sleep.h
  * @brief   Sleep mode entry (CPU halted, peripherals and clocks running)
  ******************************************************************************
  */

#ifndef __PWR_SLEEP_H__
#define __PWR_SLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Enter Sleep mode
 * @details CPU stops, peripherals continue running
 * @param   mode Sleep entry mode (WFI or WFE)
 * @retval  None
 */
void PWR_EnterSleepMode(PWR_SleepModeTypeDef mode);

/**
 * @brief   Sleep repeatedly until the given duration has elapsed
 * @details Requires SysTick to be running, since it is the wakeup source
 * @param   duration_ms Sleep duration in milliseconds
 * @retval  None
 */
void PWR_SleepFor(uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_SLEEP_H__ */
