/**
 * @file iwdg_reset.h
 * @brief Reset source reporting
 */

#ifndef IWDG_RESET_H
#define IWDG_RESET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iwdg_types.h"

/**
 * @brief   Report whether the last reset came from the independent watchdog
 * @retval  bool true if the IWDG reset flag is set
 * @note    The flag survives every reset except a power on, so read it early
 *          in startup and before clearing the flags.
 */
bool IWDG_WasResetSource(void);

/**
 * @brief   Clear the RCC reset flags
 * @note    RCC offers a single RMVF bit, so this clears every reset flag:
 *          pin, power on, software, independent and window watchdog, low
 *          power and brown out. Read all the flags of interest first.
 */
void IWDG_ClearResetFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* IWDG_RESET_H */
