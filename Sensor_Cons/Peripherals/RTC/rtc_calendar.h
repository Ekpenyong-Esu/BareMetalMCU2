/**
 ******************************************************************************
 * @file    rtc_calendar.h
 * @author  Mahonri
 * @brief   RTC calendar access: time and date get/set
 ******************************************************************************
 */

#ifndef RTC_CALENDAR_H
#define RTC_CALENDAR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtc_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Set the RTC current time
 * @param  sTime: Pointer to Time structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetTime(const RTC_Time_t *sTime);

/**
 * @brief  Get the RTC current time
 * @note   Reading the time locks the calendar shadow registers until the date
 *         is read, so call RTC_GetDate() afterwards when both are needed.
 * @param  sTime: Pointer to Time structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetTime(RTC_Time_t *sTime);

/**
 * @brief  Set the RTC current date
 * @param  sDate: Pointer to date structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetDate(const RTC_Date_t *sDate);

/**
 * @brief  Get the RTC current date
 * @param  sDate: Pointer to Date structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetDate(RTC_Date_t *sDate);

#ifdef __cplusplus
}
#endif

#endif /* RTC_CALENDAR_H */
