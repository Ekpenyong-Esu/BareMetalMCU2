/**
 * @file wwdg_timing.h
 * @brief Prescaler decoding and timeout arithmetic for the Window Watchdog
 */

#ifndef WWDG_TIMING_H
#define WWDG_TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wwdg_types.h"

/**
 * @brief   Translate a WWDG_PRESCALER_x constant into its numeric divider
 * @param   prescaler WWDG_PRESCALER_1/2/4/8
 * @param   divider   Receives 1, 2, 4 or 8
 * @retval  WWDG_INVALID_PARAM for any other value
 */
WWDG_StatusTypeDef WWDG_GetPrescalerDivider(uint32_t prescaler, uint32_t *divider);

/**
 * @brief   Calculate the refresh window of a configuration
 * @details minTimeout is the delay after a refresh before the window opens,
 *          maxTimeout is the delay before the watchdog resets the MCU. Both are
 *          in microseconds and derived from the live PCLK1 frequency.
 * @param   prescaler  WWDG_PRESCALER_1/2/4/8
 * @param   counter    Counter reload value
 * @param   window     Window value
 * @param   minTimeout Optional output, may be NULL
 * @param   maxTimeout Optional output, may be NULL
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_CalculateTimeout(uint32_t prescaler, uint32_t counter, uint32_t window,
                                         uint32_t *minTimeout, uint32_t *maxTimeout);

#ifdef __cplusplus
}
#endif

#endif /* WWDG_TIMING_H */
