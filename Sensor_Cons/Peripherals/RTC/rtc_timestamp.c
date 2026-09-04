/**
 ******************************************************************************
 * @file    rtc_timestamp.c
 * @author  Mahonri
 * @brief   Conversion between the RTC calendar and Unix timestamps
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc_timestamp.h"
#include "rtc_calendar.h"
#include "stm32f4xx_hal.h" /* RTC_HOURFORMAT_24, RTC_DAYLIGHTSAVING_NONE, RTC_STOREOPERATION_RESET */

/* Private define ------------------------------------------------------------*/

/** Jan 1 1970 was a Thursday, which is weekday 4 in RTC_WeekDay_Definitions */
#define RTC_EPOCH_WEEKDAY_OFFSET 3U

#define RTC_MONTHS_PER_YEAR 12U
#define RTC_DAYS_PER_YEAR 365U
#define RTC_DAYS_PER_LEAP_YEAR 366U
#define RTC_FEBRUARY_LEAP_DAYS 29U
#define RTC_FEBRUARY 2U

/* Gregorian rule: every 4th year is leap, except centuries, except every 400th */
#define RTC_LEAP_INTERVAL_YEARS 4U
#define RTC_CENTURY_YEARS 100U
#define RTC_LEAP_CENTURY_INTERVAL_YEARS 400U

/* Private variables ---------------------------------------------------------*/

/** Days in each month of a non-leap year */
static const uint32_t rtc_daysInMonth[RTC_MONTHS_PER_YEAR] = {31, 28, 31, 30, 31, 30,
                                                              31, 31, 30, 31, 30, 31};

/* Private function prototypes -----------------------------------------------*/
static bool RTC_IsLeapYear(uint32_t year);
static uint32_t RTC_DaysInMonth(uint32_t month, uint32_t year);

/* Public functions ----------------------------------------------------------*/

uint32_t RTC_DateTimeToTimestamp(const RTC_Date_t *sDate, const RTC_Time_t *sTime) {
    uint32_t year = 0;
    uint32_t days = 0;

    if (sDate == NULL || sTime == NULL) {
        return 0;
    }

    if (sDate->Month < 1U || sDate->Month > RTC_MONTHS_PER_YEAR || sDate->Date < 1U) {
        return 0;
    }

    year = RTC_CALENDAR_BASE_YEAR + sDate->Year;

    /* Whole years since the epoch */
    for (uint32_t pastYear = RTC_EPOCH_BASE_YEAR; pastYear < year; pastYear++) {
        days += RTC_IsLeapYear(pastYear) ? RTC_DAYS_PER_LEAP_YEAR : RTC_DAYS_PER_YEAR;
    }

    /* Whole months of the current year */
    for (uint32_t pastMonth = 1U; pastMonth < sDate->Month; pastMonth++) {
        days += RTC_DaysInMonth(pastMonth, year);
    }

    /* Whole days of the current month */
    days += (uint32_t)sDate->Date - 1U;

    return (days * RTC_SECONDS_PER_DAY) + ((uint32_t)sTime->Hours * RTC_SECONDS_PER_HOUR) +
           ((uint32_t)sTime->Minutes * RTC_SECONDS_PER_MINUTE) + sTime->Seconds;
}

void RTC_TimestampToDateTime(uint32_t timestamp, RTC_Date_t *sDate, RTC_Time_t *sTime) {
    uint32_t days = 0;
    uint32_t seconds = 0;
    uint32_t year = 0;
    uint32_t month = 0;

    if (sDate == NULL || sTime == NULL) {
        return;
    }

    days = timestamp / RTC_SECONDS_PER_DAY;
    seconds = timestamp % RTC_SECONDS_PER_DAY;

    sTime->Hours = (uint8_t)(seconds / RTC_SECONDS_PER_HOUR);
    sTime->Minutes = (uint8_t)((seconds % RTC_SECONDS_PER_HOUR) / RTC_SECONDS_PER_MINUTE);
    sTime->Seconds = (uint8_t)(seconds % RTC_SECONDS_PER_MINUTE);
    sTime->TimeFormat = RTC_HOURFORMAT_24;
    sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime->StoreOperation = RTC_STOREOPERATION_RESET;

    /* The weekday only depends on the day count, so derive it before days is
       consumed by the year and month loops below. */
    sDate->WeekDay = (uint8_t)(((days + RTC_EPOCH_WEEKDAY_OFFSET) % 7U) + 1U);

    year = RTC_EPOCH_BASE_YEAR;
    for (;;) {
        const uint32_t daysThisYear = RTC_IsLeapYear(year) ? 366U : 365U;

        if (days < daysThisYear) {
            break;
        }

        days -= daysThisYear;
        year++;
    }

    for (month = 1U; month < RTC_MONTHS_PER_YEAR; month++) {
        const uint32_t daysThisMonth = RTC_DaysInMonth(month, year);

        if (days < daysThisMonth) {
            break;
        }

        days -= daysThisMonth;
    }

    sDate->Year = (uint8_t)(year - RTC_CALENDAR_BASE_YEAR);
    sDate->Month = (uint8_t)month;
    sDate->Date = (uint8_t)(days + 1U);
}

uint32_t RTC_GetTimestamp(void) {
    RTC_Date_t sDate;
    RTC_Time_t sTime;

    /* The calendar shadow registers stay locked between a time read and the
       following date read, so the time must be fetched first. */
    if (RTC_GetTime(&sTime) != RTC_STATUS_OK || RTC_GetDate(&sDate) != RTC_STATUS_OK) {
        return 0;
    }

    return RTC_DateTimeToTimestamp(&sDate, &sTime);
}

RTC_StatusTypeDef RTC_SetTimestamp(uint32_t timestamp) {
    RTC_Date_t sDate;
    RTC_Time_t sTime;

    RTC_TimestampToDateTime(timestamp, &sDate, &sTime);

    if (RTC_SetDate(&sDate) != RTC_STATUS_OK || RTC_SetTime(&sTime) != RTC_STATUS_OK) {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Check whether a year has 366 days
 */
static bool RTC_IsLeapYear(uint32_t year) {
    return ((year % RTC_LEAP_INTERVAL_YEARS) == 0U && (year % RTC_CENTURY_YEARS) != 0U) ||
           ((year % RTC_LEAP_CENTURY_INTERVAL_YEARS) == 0U);
}

/**
 * @brief  Number of days in a given month of a given year
 * @param  month: 1..12
 * @param  year: Full year, e.g. 2025
 */
static uint32_t RTC_DaysInMonth(uint32_t month, uint32_t year) {
    if (month == RTC_FEBRUARY && RTC_IsLeapYear(year)) {
        return RTC_FEBRUARY_LEAP_DAYS;
    }

    return rtc_daysInMonth[month - 1U];
}
