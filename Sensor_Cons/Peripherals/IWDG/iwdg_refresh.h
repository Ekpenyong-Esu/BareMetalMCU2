/**
 * @file iwdg_refresh.h
 * @brief Watchdog counter refresh
 */

#ifndef IWDG_REFRESH_H
#define IWDG_REFRESH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iwdg_types.h"

/**
 * @brief   Reload the watchdog counter
 * @retval  IWDG_StatusTypeDef IWDG_NOT_READY if the watchdog was never started
 * @note    Must be called more often than the configured period, and with
 *          margin: the LSI can run up to 47 percent fast.
 */
IWDG_StatusTypeDef IWDG_Refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* IWDG_REFRESH_H */
