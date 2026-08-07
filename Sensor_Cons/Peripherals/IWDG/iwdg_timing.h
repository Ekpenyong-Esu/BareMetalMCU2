/**
 * @file iwdg_timing.h
 * @brief Prescaler, reload and timeout arithmetic
 */

#ifndef IWDG_TIMING_H
#define IWDG_TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iwdg_types.h"

/**
 * @brief   Translate a HAL prescaler constant into its divider
 * @param   prescaler IWDG_PRESCALER_x value
 * @param   divider Destination for the divider
 * @retval  IWDG_StatusTypeDef IWDG_INVALID_PARAM for an unknown prescaler
 */
IWDG_StatusTypeDef IWDG_GetPrescalerDivider(uint32_t prescaler, uint32_t *divider);

/**
 * @brief   Compute the watchdog period of a prescaler and reload pair
 * @param   prescaler IWDG_PRESCALER_x value
 * @param   reload Reload value
 * @param   timeout_ms Destination for the period in milliseconds
 * @retval  IWDG_StatusTypeDef IWDG_OK on success
 * @note    A period lasts (reload + 1) ticks, since the counter resets as it
 *          underflows past zero.
 */
IWDG_StatusTypeDef IWDG_CalculateTimeout(uint32_t prescaler, uint32_t reload,
                                         uint32_t *timeout_ms);

/**
 * @brief   Choose the prescaler and reload that cover a requested period
 * @param   timeout_ms Requested period in milliseconds
 * @param   prescaler Destination for the chosen prescaler
 * @param   reload Destination for the chosen reload
 * @retval  IWDG_StatusTypeDef IWDG_OK on success
 * @note    Rounds up, so the resulting period is never shorter than requested;
 *          rounding down would make the watchdog fire early.
 */
IWDG_StatusTypeDef IWDG_CalculatePrescalerReload(uint32_t timeout_ms,
                                                 uint32_t *prescaler, uint32_t *reload);

#ifdef __cplusplus
}
#endif

#endif /* IWDG_TIMING_H */
