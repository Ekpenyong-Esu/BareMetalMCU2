/**
 * @file iwdg_core.h
 * @brief Independent watchdog initialization
 *
 * @note Once started the IWDG cannot be stopped by software; only a reset
 *       clears it. There is deliberately no deinitialization entry point.
 */

#ifndef IWDG_CORE_H
#define IWDG_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iwdg_types.h"

/**
 * @brief   Start the watchdog with the default one second period
 * @retval  IWDG_StatusTypeDef IWDG_OK on success
 */
IWDG_StatusTypeDef IWDG_Init(void);

/**
 * @brief   Start the watchdog with an explicit prescaler and reload
 * @param   config Requested configuration
 * @retval  IWDG_StatusTypeDef IWDG_OK on success
 */
IWDG_StatusTypeDef IWDG_Init_Custom(const IWDG_ConfigTypeDef *config);

/**
 * @brief   Start the watchdog with a period given in milliseconds
 * @param   timeout_ms Period from 1 to IWDG_TIMEOUT_MAX_MS
 * @retval  IWDG_StatusTypeDef IWDG_OK on success
 * @note    The period is rounded up to the next representable value.
 */
IWDG_StatusTypeDef IWDG_Init_TimeoutMs(uint32_t timeout_ms);

/**
 * @brief   Report whether the watchdog has been started
 * @retval  bool true once an initialization has succeeded
 */
bool IWDG_IsInitialized(void);

/**
 * @brief   Access the HAL handle
 * @retval  IWDG_HandleTypeDef* NULL until an initialization has succeeded
 * @note    Returning NULL rather than an unpopulated handle stops callers
 *          dereferencing a null Instance pointer.
 */
IWDG_HandleTypeDef *IWDG_GetHandle(void);

/**
 * @brief   Read the configuration currently programmed
 * @param   config Destination
 * @retval  IWDG_StatusTypeDef IWDG_NOT_READY before initialization
 */
IWDG_StatusTypeDef IWDG_GetConfig(IWDG_ConfigTypeDef *config);

/**
 * @brief   Read the period currently programmed, in milliseconds
 * @param   timeout_ms Destination
 * @retval  IWDG_StatusTypeDef IWDG_NOT_READY before initialization
 */
IWDG_StatusTypeDef IWDG_GetTimeoutMs(uint32_t *timeout_ms);

/**
 * @brief   Describe a status code
 * @param   status Status code
 * @retval  const char* Static description
 */
const char *IWDG_GetStatusString(IWDG_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* IWDG_CORE_H */
