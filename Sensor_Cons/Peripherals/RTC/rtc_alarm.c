/**
 ******************************************************************************
 * @file    rtc_alarm.c
 * @author  Mahonri
 * @brief   RTC alarm configuration, interrupt dispatch and event callbacks
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc_alarm.h"
#include "rtc_core.h"

/* Public functions ----------------------------------------------------------*/

RTC_StatusTypeDef RTC_SetAlarm(const RTC_Alarm_t* sAlarm)
{
    RTC_AlarmTypeDef hal_alarm = {0};

    if (sAlarm == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    hal_alarm.AlarmTime.Hours = sAlarm->AlarmTime.Hours;
    hal_alarm.AlarmTime.Minutes = sAlarm->AlarmTime.Minutes;
    hal_alarm.AlarmTime.Seconds = sAlarm->AlarmTime.Seconds;
    hal_alarm.AlarmTime.TimeFormat = sAlarm->AlarmTime.TimeFormat;
    hal_alarm.AlarmMask = sAlarm->AlarmMask;
    hal_alarm.AlarmSubSecondMask = sAlarm->AlarmSubSecondMask;
    hal_alarm.AlarmDateWeekDaySel = sAlarm->AlarmDateWeekDaySel;
    hal_alarm.AlarmDateWeekDay = sAlarm->AlarmDateWeekDay;
    hal_alarm.Alarm = sAlarm->Alarm;

    if (HAL_RTC_SetAlarm_IT(RTC_GetHandle(), &hal_alarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

RTC_StatusTypeDef RTC_GetAlarm(RTC_Alarm_t* sAlarm, uint32_t Alarm)
{
    RTC_AlarmTypeDef hal_alarm = {0};

    if (sAlarm == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    hal_alarm.Alarm = Alarm;

    if (HAL_RTC_GetAlarm(RTC_GetHandle(), &hal_alarm, Alarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    sAlarm->AlarmTime.Hours = hal_alarm.AlarmTime.Hours;
    sAlarm->AlarmTime.Minutes = hal_alarm.AlarmTime.Minutes;
    sAlarm->AlarmTime.Seconds = hal_alarm.AlarmTime.Seconds;
    sAlarm->AlarmTime.TimeFormat = hal_alarm.AlarmTime.TimeFormat;
    sAlarm->AlarmMask = hal_alarm.AlarmMask;
    sAlarm->AlarmSubSecondMask = hal_alarm.AlarmSubSecondMask;
    sAlarm->AlarmDateWeekDaySel = hal_alarm.AlarmDateWeekDaySel;
    sAlarm->AlarmDateWeekDay = hal_alarm.AlarmDateWeekDay;
    sAlarm->Alarm = hal_alarm.Alarm;

    return RTC_STATUS_OK;
}

RTC_StatusTypeDef RTC_DisableAlarm(uint32_t Alarm)
{
    if (HAL_RTC_DeactivateAlarm(RTC_GetHandle(), Alarm) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

__weak void RTC_AlarmCallback(uint32_t Alarm)
{
    /* Override this function in the application to react to alarms */
    UNUSED(Alarm);
}

void RTC_ISR_Dispatch(void)
{
    HAL_RTC_AlarmIRQHandler(RTC_GetHandle());
}

/* HAL callbacks -------------------------------------------------------------*/

/**
 * @brief  RTC Alarm A callback
 * @param  hrtc: RTC handle
 * @retval None
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef* hrtc)
{
    UNUSED(hrtc);
    RTC_AlarmCallback(RTC_ALARM_A);
}

/**
 * @brief  RTC Alarm B callback
 * @param  hrtc: RTC handle
 * @retval None
 */
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef* hrtc)
{
    UNUSED(hrtc);
    RTC_AlarmCallback(RTC_ALARM_B);
}
