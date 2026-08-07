/**
 ******************************************************************************
 * @file    rtc_core.h
 * @author  Mahonri
 * @brief   RTC lifecycle: initialisation, de-initialisation and handle access
 ******************************************************************************
 */

#ifndef __RTC_CORE_H
#define __RTC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtc_types.h"
#include "main.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initialize the RTC peripheral and its clock source
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_Init(void);

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
RTC_HandleTypeDef* RTC_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_CORE_H */
