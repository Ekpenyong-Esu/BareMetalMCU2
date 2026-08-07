/**
 ******************************************************************************
 * @file    rtc_alarm.h
 * @author  Mahonri
 * @brief   RTC alarm configuration and event callback
 ******************************************************************************
 */

#ifndef __RTC_ALARM_H
#define __RTC_ALARM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtc_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Set the specified RTC Alarm and enable its interrupt
 * @param  sAlarm: Pointer to Alarm structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetAlarm(const RTC_Alarm_t* sAlarm);

/**
 * @brief  Get the specified RTC Alarm
 * @param  sAlarm: Pointer to Alarm structure
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetAlarm(RTC_Alarm_t* sAlarm, uint32_t Alarm);

/**
 * @brief  Disable the specified RTC Alarm
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_DisableAlarm(uint32_t Alarm);

/**
 * @brief  Callback invoked when an RTC alarm fires
 * @note   Weak; override it in the application to react to alarms.
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval None
 */
void RTC_AlarmCallback(uint32_t Alarm);

/**
 * @brief  RTC Alarm interrupt dispatcher
 * @note   Called from RTC_Alarm_IRQHandler() in Core/Src/stm32f4xx_it.c.
 *         Keeps the RTC handle private while letting the interrupt layer
 *         own the vector-table entry point.
 * @retval None
 */
void RTC_ISR_Dispatch(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_ALARM_H */
