/**
 ******************************************************************************
 * @file    rtc_format.h
 * @author  Mahonri
 * @brief   Human readable rendering of RTC time and date values
 ******************************************************************************
 */

#ifndef __RTC_FORMAT_H
#define __RTC_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtc_types.h"

/* Exported constants --------------------------------------------------------*/

/** Minimum buffer size for RTC_FormatTimeString(), e.g. "11:59:59 PM" */
#define RTC_TIME_STRING_SIZE    12U

/** Minimum buffer size for RTC_FormatDateString(), e.g. "Mon 31/12/2099" */
#define RTC_DATE_STRING_SIZE    16U

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Format time as string
 * @param  sTime: Pointer to Time structure
 * @param  buffer: Buffer to store formatted time string
 * @param  size: Size of the buffer, see RTC_TIME_STRING_SIZE
 * @retval None
 */
void RTC_FormatTimeString(const RTC_Time_t* sTime, char* buffer, size_t size);

/**
 * @brief  Format date as string
 * @param  sDate: Pointer to Date structure
 * @param  buffer: Buffer to store formatted date string
 * @param  size: Size of the buffer, see RTC_DATE_STRING_SIZE
 * @retval None
 */
void RTC_FormatDateString(const RTC_Date_t* sDate, char* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_FORMAT_H */
