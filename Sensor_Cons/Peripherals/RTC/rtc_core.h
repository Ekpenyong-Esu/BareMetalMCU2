/**
 ******************************************************************************
 * @file    rtc_core.h
 * @author  Mahonri
 * @brief   RTC lifecycle: initialisation, de-initialisation and handle access
 * @details The RTC is a singleton peripheral, so the driver owns its HAL
 *          handle. The application still chooses the clock source through
 *          RTC_ConfigTypeDef; nothing about the board is assumed here.
 ******************************************************************************
 */

#ifndef RTC_CORE_H
#define RTC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtc_types.h"
#include "stm32f4xx_hal.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Nominal configuration for a clock source: 24-hour format and the
 *         prescalers that divide it to 1 Hz
 * @param  source Oscillator the application wants to feed the RTC from
 * @note   For RTC_CLOCK_HSE_DIV the prescalers and divider are left at 0
 *         because they depend on the crystal; the caller must fill them.
 * @retval RTC_ConfigTypeDef: Copy it and override only what differs
 */
RTC_ConfigTypeDef RTC_ConfigForClockSource(RTC_ClockSourceTypeDef source);

/**
 * @brief  Default configuration: LSI, since it needs no external part
 * @retval RTC_ConfigTypeDef: Same as RTC_ConfigForClockSource(RTC_CLOCK_LSI)
 */
RTC_ConfigTypeDef RTC_ConfigDefault(void);

/**
 * @brief  Initialize the RTC peripheral and its clock source with the defaults
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_Init(void);

/**
 * @brief  Initialize the RTC peripheral with a caller-supplied configuration
 * @param  config Clock source, hour format and prescalers to apply
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_Init_Custom(const RTC_ConfigTypeDef *config);

/**
 * @brief  Deinitialize the RTC peripheral
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_DeInit(void);

/**
 * @brief  Access the RTC HAL handle
 * @note   Intended for the other RTC modules only; application code should use
 *         the calendar, alarm and timestamp APIs instead.
 * @retval Pointer to the driver's RTC handle
 */
RTC_HandleTypeDef *RTC_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* RTC_CORE_H */
