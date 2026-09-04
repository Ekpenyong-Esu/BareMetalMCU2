/**
 * @file ts_core.h
 * @brief Touchscreen lifecycle and controller bring-up
 */

#ifndef TS_CORE_H
#define TS_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_types.h"

/**
 * @brief Bring up the STMPE811 and the handle that represents it
 * @param hts Handle to initialise
 * @param config Bus, INT pin and display size supplied by the application;
 *               the bus must already be open
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Init(TS_HandleTypeDef *hts, const TS_ConfigTypeDef *config);

/**
 * @brief Power down the controller and invalidate the handle
 * @param hts Touchscreen handle
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_DeInit(TS_HandleTypeDef *hts);

/**
 * @brief Store a configuration in the handle
 * @param hts Touchscreen handle
 * @param config Configuration to apply
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Configure(TS_HandleTypeDef *hts, const TS_ConfigTypeDef *config);

/**
 * @brief Software-reset the STMPE811
 * @param hts Touchscreen handle
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Reset(TS_HandleTypeDef *hts);

/**
 * @brief Starting point for a configuration
 * @details Interrupts are on; the bus, INT pin and display size are left
 *          blank because only the application knows them.
 * @return TS_ConfigTypeDef Default configuration
 */
TS_ConfigTypeDef TS_GetDefaultConfig(void);

/**
 * @brief Attach callbacks to a handle
 * @param hts Touchscreen handle
 * @param touch_callback Called when a touch is detected, may be NULL
 * @param release_callback Called when a touch is released, may be NULL
 * @param gesture_callback Called when a gesture is recognised, may be NULL
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_RegisterCallbacks(TS_HandleTypeDef *hts, void (*touch_callback)(void),
                                      void (*release_callback)(void),
                                      void (*gesture_callback)(TS_GestureTypeDef));

/**
 * @brief Attach the callback run from interrupt context on every touch edge
 * @details Meant for wake-from-idle bookkeeping; it runs before the STMPE811
 *          has been serviced, so it must not touch the bus.
 * @param hts Touchscreen handle
 * @param activity_callback Called from TS_EXTI_Callback, may be NULL
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_SetActivityCallback(TS_HandleTypeDef *hts, void (*activity_callback)(void));

#ifdef __cplusplus
}
#endif

#endif /* TS_CORE_H */
