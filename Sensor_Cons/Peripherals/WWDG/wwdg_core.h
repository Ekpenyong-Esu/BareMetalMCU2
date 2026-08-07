/**
 * @file wwdg_core.h
 * @brief Window Watchdog lifetime, handle ownership and reset-source reporting
 */

#ifndef WWDG_CORE_H
#define WWDG_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wwdg_types.h"

/**
 * @brief   Initialize the Window Watchdog with the default configuration
 * @warning The watchdog is armed as soon as this returns and the WDGA bit
 *          cannot be cleared by software: from here on the application must
 *          refresh within the window or the MCU is reset.
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Init(void);

/**
 * @brief   Initialize the Window Watchdog with a custom configuration
 * @warning Arms the watchdog irreversibly, see WWDG_Init().
 * @param   config Configuration to apply
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Init_Custom(const WWDG_ConfigTypeDef *config);

/**
 * @brief   Fill a configuration structure with the driver defaults
 * @param   config Structure to populate
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_GetDefaultConfig(WWDG_ConfigTypeDef *config);

/**
 * @brief   Release the driver state
 * @details The hardware watchdog keeps running; only a reset stops it. This
 *          masks the early wakeup interrupt and drops the driver state so the
 *          peripheral can be reconfigured, and reports WWDG_NOT_SUPPORTED to
 *          make clear the watchdog itself was not stopped.
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_DeInit(void);

/**
 * @brief   Check whether the driver has been initialized
 * @retval  bool True once WWDG_Init/WWDG_Init_Custom succeeded
 */
bool WWDG_IsInitialized(void);

/**
 * @brief   Access the HAL handle
 * @retval  WWDG_HandleTypeDef* NULL until the driver is initialized
 */
WWDG_HandleTypeDef *WWDG_GetHandle(void);

/**
 * @brief   Check if the last reset was caused by the Window Watchdog
 * @retval  bool True if WWDG caused the reset
 */
bool WWDG_WasResetSource(void);

/**
 * @brief   Clear the reset source flags
 * @warning The RMVF bit clears every reset flag at once, so any other module
 *          that needs the reset cause must read it before this is called.
 */
void WWDG_ClearResetFlag(void);

/**
 * @brief   Convert a status code to a human readable string
 * @param   status Status code
 * @retval  const char* Status description
 */
const char *WWDG_GetStatusString(WWDG_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* WWDG_CORE_H */
