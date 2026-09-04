/**
 * @file rtc_types.h
 * @brief Types for the real-time clock (RTC)
 * @details The RTC is like a small clock inside the board. It keeps time
 *          and date even when the board sleeps. It can also wake the board
 *          up with an alarm.
 *
 * How it works (in simple words):
 *  - Set the time and date once.
 *  - The clock keeps ticking on its own.
 *  - Read time/date anytime, or set an alarm to wake up.
 */

#ifndef RTC_TYPES_H
#define RTC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> /* size_t */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Oscillator that feeds the RTC
 * @note  The application chooses: only it knows whether a 32.768 kHz crystal
 *        is fitted (LSE) or whether the board must fall back to the internal
 *        RC oscillator (LSI, accurate to a few percent).
 */
typedef enum {
    RTC_CLOCK_LSI = 0, /*!< Internal ~32 kHz RC oscillator */
    RTC_CLOCK_LSE,     /*!< External 32.768 kHz crystal */
    RTC_CLOCK_HSE_DIV  /*!< HSE divided by HseRtcClockSelection; HSE must already run */
} RTC_ClockSourceTypeDef;

/**
 * @brief RTC start-up configuration
 * @note  The prescalers are exposed because LSI drifts and trimming them is
 *        the way to correct it. RTC_ConfigForClockSource() fills the nominal
 *        pair for LSI and LSE; for HSE the divider and both prescalers are
 *        the caller's, since they depend on the crystal fitted.
 */
typedef struct {
    RTC_ClockSourceTypeDef ClockSource; /*!< Oscillator feeding the RTC */
    uint32_t
        HseRtcClockSelection; /*!< RCC_RTCCLKSOURCE_HSE_DIVx; only read for RTC_CLOCK_HSE_DIV */
    uint32_t HourFormat;      /*!< RTC_HOURFORMAT_12 or RTC_HOURFORMAT_24 */
    uint32_t AsynchPrediv;    /*!< Asynchronous prescaler, 0..0x7F */
    uint32_t SynchPrediv;     /*!< Synchronous prescaler, 0..0x7FFF */
} RTC_ConfigTypeDef;

/**
 * @brief RTC Time structure definition
 */
typedef struct {
    uint8_t Hours; /*!< RTC Time Hour.
                        0..12 when RTC_HourFormat_12 is selected,
                        0..23 when RTC_HourFormat_24 is selected */

    uint8_t Minutes; /*!< RTC Time Minutes, 0..59 */

    uint8_t Seconds; /*!< RTC Time Seconds, 0..59 */

    uint8_t TimeFormat; /*!< RTC AM/PM Time.
                             A value of @ref RTC_AM_PM_Definitions */

    uint32_t DayLightSaving; /*!< DayLight Save Operation.
                                  A value of @ref RTC_DayLightSaving_Definitions */

    uint32_t StoreOperation; /*!< RTC_StoreOperation value written to the BCK bit
                                  in the CR register.
                                  A value of @ref RTC_StoreOperation_Definitions */
} RTC_Time_t;

/**
 * @brief RTC Date structure definition
 */
typedef struct {
    uint8_t WeekDay; /*!< RTC Date WeekDay.
                          A value of @ref RTC_WeekDay_Definitions (1..7) */

    uint8_t Month; /*!< RTC Date Month.
                        A value of @ref RTC_Month_Date_Definitions (1..12) */

    uint8_t Date; /*!< RTC Date, 1..31 */

    uint8_t Year; /*!< RTC Date Year, 0..99 (offset from 2000) */
} RTC_Date_t;

/**
 * @brief RTC Alarm structure definition
 */
typedef struct {
    RTC_Time_t AlarmTime; /*!< RTC Alarm Time members */

    uint32_t AlarmMask; /*!< RTC Alarm Masks.
                             A value of @ref RTC_AlarmMask_Definitions */

    uint32_t AlarmSubSecondMask; /*!< RTC Alarm SubSeconds Masks.
                                      A value of @ref RTC_Alarm_Sub_Seconds_Masks_Definitions */

    uint32_t AlarmDateWeekDaySel; /*!< Selects Date or WeekDay matching.
                                       A value of @ref RTC_AlarmDateWeekDay_Definitions */

    uint8_t AlarmDateWeekDay; /*!< RTC Alarm Date/WeekDay.
                                   1..31 when Date matching is selected, otherwise
                                   a value of @ref RTC_WeekDay_Definitions */

    uint32_t Alarm; /*!< Selected alarm.
                         A value of @ref RTC_Alarms_Definitions */
} RTC_Alarm_t;

/**
 * @brief RTC operation status enumeration
 */
typedef enum {
    RTC_STATUS_OK = 0x00U,
    RTC_STATUS_ERROR = 0x01U,
    RTC_STATUS_BUSY = 0x02U,
    RTC_STATUS_TIMEOUT = 0x03U
} RTC_StatusTypeDef;

/* Exported constants --------------------------------------------------------*/

/** Seconds in one day */
#define RTC_SECONDS_PER_DAY 86400U

/** Seconds in one hour */
#define RTC_SECONDS_PER_HOUR 3600U

/** Seconds in one minute */
#define RTC_SECONDS_PER_MINUTE 60U

/** Base year of the RTC calendar (the Year field is an offset from it) */
#define RTC_CALENDAR_BASE_YEAR 2000U

/** Base year of the Unix epoch */
#define RTC_EPOCH_BASE_YEAR 1970U

#ifdef __cplusplus
}
#endif

#endif /* RTC_TYPES_H */
