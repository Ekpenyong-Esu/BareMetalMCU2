/**
 ******************************************************************************
 * @file    rtc_timestamp.h
 * @author  Mahonri
 * @brief   Conversion between the RTC calendar and Unix timestamps
 ******************************************************************************
 */

#ifndef RTC_TIMESTAMP_H
#define RTC_TIMESTAMP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtc_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Convert a calendar date and time to a Unix timestamp
 * @param  sDate: Pointer to Date structure
 * @param  sTime: Pointer to Time structure
 * @retval uint32_t: Seconds since Jan 1 1970, or 0 on invalid input
 */
uint32_t RTC_DateTimeToTimestamp(const RTC_Date_t *sDate, const RTC_Time_t *sTime);

/**
 * @brief  Convert a Unix timestamp to a calendar date and time
 * @param  timestamp: Seconds since Jan 1 1970
 * @param  sDate: Pointer to Date structure to fill
 * @param  sTime: Pointer to Time structure to fill
 * @retval None
 */
void RTC_TimestampToDateTime(uint32_t timestamp, RTC_Date_t *sDate, RTC_Time_t *sTime);

/**
 * @brief  Read the RTC as a Unix timestamp
 * @retval uint32_t: Seconds since Jan 1 1970, or 0 if the RTC cannot be read
 */
uint32_t RTC_GetTimestamp(void);

/**
 * @brief  Set the RTC from a Unix timestamp
 * @param  timestamp: Seconds since Jan 1 1970
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetTimestamp(uint32_t timestamp);

#ifdef __cplusplus
}
#endif

#endif /* RTC_TIMESTAMP_H */
