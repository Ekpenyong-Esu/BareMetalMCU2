/**
 ******************************************************************************
 * @file    rtc_format.c
 * @author  Mahonri
 * @brief   Human readable rendering of RTC time and date values
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc_format.h"
#include "main.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/

/** Indexed by RTC_WeekDay_Definitions (1..7); slot 0 covers an unset field */
static const char* const rtc_weekdayNames[8] = {
    "", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

/* Public functions ----------------------------------------------------------*/

void RTC_FormatTimeString(const RTC_Time_t* sTime, char* buffer, size_t size)
{
    if (sTime == NULL || buffer == NULL || size == 0U)
    {
        return;
    }

    if (sTime->TimeFormat == RTC_HOURFORMAT12_PM || sTime->TimeFormat == RTC_HOURFORMAT12_AM)
    {
        (void)snprintf(buffer, size, "%02u:%02u:%02u %s",
                       (unsigned)sTime->Hours, (unsigned)sTime->Minutes,
                       (unsigned)sTime->Seconds,
                       (sTime->TimeFormat == RTC_HOURFORMAT12_PM) ? "PM" : "AM");
    }
    else
    {
        (void)snprintf(buffer, size, "%02u:%02u:%02u",
                       (unsigned)sTime->Hours, (unsigned)sTime->Minutes,
                       (unsigned)sTime->Seconds);
    }
}

void RTC_FormatDateString(const RTC_Date_t* sDate, char* buffer, size_t size)
{
    uint8_t weekday;

    if (sDate == NULL || buffer == NULL || size == 0U)
    {
        return;
    }

    /* Guard the lookup: a caller-supplied structure may hold any value */
    weekday = (sDate->WeekDay < 8U) ? sDate->WeekDay : 0U;

    (void)snprintf(buffer, size, "%s %02u/%02u/%04u",
                   rtc_weekdayNames[weekday],
                   (unsigned)sDate->Date, (unsigned)sDate->Month,
                   (unsigned)(RTC_CALENDAR_BASE_YEAR + sDate->Year));
}
