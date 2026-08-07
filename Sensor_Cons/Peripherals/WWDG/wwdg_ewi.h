/**
 * @file wwdg_ewi.h
 * @brief Early wakeup interrupt handling for the Window Watchdog
 */

#ifndef WWDG_EWI_H
#define WWDG_EWI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wwdg_types.h"

/**
 * @brief   Register the early wakeup callback
 * @details The callback runs from interrupt context roughly one counter tick
 *          before the reset. It does not refresh the watchdog: call
 *          WWDG_Refresh() from the callback if the system is to survive.
 * @param   callback Function to call, or NULL to unregister
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_RegisterEWICallback(WWDG_EWI_Callback_t callback);

/**
 * @brief   Enable the early wakeup interrupt and unmask it in the NVIC
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_EnableEWI(void);

/**
 * @brief   Mask the early wakeup interrupt in the NVIC
 * @note    The EWI bit in CFR is cleared only by a reset, so the flag keeps
 *          being set in hardware; only delivery is suppressed.
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_DisableEWI(void);

/**
 * @brief   Check whether the early wakeup interrupt is enabled
 * @retval  bool True when EWI is set and unmasked
 */
bool WWDG_IsEWIEnabled(void);

/**
 * @brief   Early wakeup interrupt service routine
 * @details Call from WWDG_IRQHandler() in stm32f4xx_it.c.
 */
void WWDG_EWI_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* WWDG_EWI_H */
