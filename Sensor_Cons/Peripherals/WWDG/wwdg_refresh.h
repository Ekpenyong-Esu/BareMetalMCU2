/**
 * @file wwdg_refresh.h
 * @brief Counter refresh and window state of the Window Watchdog
 */

#ifndef WWDG_REFRESH_H
#define WWDG_REFRESH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wwdg_types.h"

/**
 * @brief   Refresh the watchdog counter
 * @details Refuses to write while the counter is still above the window value,
 *          because that write would itself trigger a reset.
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Refresh(void);

/**
 * @brief   Refresh and adopt a new counter reload value
 * @param   counter New counter value, WWDG_COUNTER_MIN..WWDG_COUNTER_MAX
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_RefreshWithCounter(uint32_t counter);

/**
 * @brief   Read the live down-counter value
 * @retval  uint32_t Counter value, or 0 when the driver is not initialized
 */
uint32_t WWDG_GetCounter(void);

/**
 * @brief   Read the window value programmed in the peripheral
 * @retval  uint32_t Window value, or 0 when the driver is not initialized
 */
uint32_t WWDG_GetWindow(void);

/**
 * @brief   Check whether a refresh is currently allowed
 * @retval  bool True when the counter has dropped into the window
 */
bool WWDG_IsInWindow(void);

#ifdef __cplusplus
}
#endif

#endif /* WWDG_REFRESH_H */
