/**
 ******************************************************************************
 * @file    rtc_calendar.c
 * @author  Mahonri
 * @brief   RTC calendar access: time and date get/set
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc_calendar.h"
#include "rtc_core.h"

/* Public functions ----------------------------------------------------------*/

RTC_StatusTypeDef RTC_SetTime(const RTC_Time_t* sTime)
{
    RTC_TimeTypeDef hal_time = {0};
    RTC_HandleTypeDef *handle = NULL;

    if (sTime == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    hal_time.Hours = sTime->Hours;
    hal_time.Minutes = sTime->Minutes;
    hal_time.Seconds = sTime->Seconds;
    hal_time.TimeFormat = sTime->TimeFormat;
    hal_time.DayLightSaving = sTime->DayLightSaving;
    hal_time.StoreOperation = sTime->StoreOperation;

    handle = RTC_GetHandle();
    if (HAL_RTC_SetTime(handle, &hal_time, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

RTC_StatusTypeDef RTC_GetTime(RTC_Time_t* sTime)
{
    RTC_TimeTypeDef hal_time = {0};
    RTC_HandleTypeDef *handle = NULL;

    if (sTime == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    handle = RTC_GetHandle();
    if (HAL_RTC_GetTime(handle, &hal_time, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    sTime->Hours = hal_time.Hours;
    sTime->Minutes = hal_time.Minutes;
    sTime->Seconds = hal_time.Seconds;
    sTime->TimeFormat = hal_time.TimeFormat;
    sTime->DayLightSaving = hal_time.DayLightSaving;
    sTime->StoreOperation = hal_time.StoreOperation;

    return RTC_STATUS_OK;
}

RTC_StatusTypeDef RTC_SetDate(const RTC_Date_t* sDate)
{
    RTC_DateTypeDef hal_date = {0};
    RTC_HandleTypeDef *handle = NULL;

    if (sDate == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    hal_date.WeekDay = sDate->WeekDay;
    hal_date.Month = sDate->Month;
    hal_date.Date = sDate->Date;
    hal_date.Year = sDate->Year;

    handle = RTC_GetHandle();
    if (HAL_RTC_SetDate(handle, &hal_date, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

RTC_StatusTypeDef RTC_GetDate(RTC_Date_t* sDate)
{
    RTC_DateTypeDef hal_date = {0};
    RTC_HandleTypeDef *handle = NULL;

    if (sDate == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    handle = RTC_GetHandle();
    if (HAL_RTC_GetDate(handle, &hal_date, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    sDate->WeekDay = hal_date.WeekDay;
    sDate->Month = hal_date.Month;
    sDate->Date = hal_date.Date;
    sDate->Year = hal_date.Year;

    return RTC_STATUS_OK;
}
