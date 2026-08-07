/**
 * @file ts_core.h
 * @brief Touchscreen lifecycle, controller bring-up and handle registry
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
 * @param hi2c I2C handle to use; an uninitialised instance makes the driver
 *             fall back to the shared I2C3 handle
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Init(TS_HandleTypeDef *hts, I2C_HandleTypeDef *hi2c);

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
TS_StatusTypeDef TS_Configure(TS_HandleTypeDef *hts, TS_ConfigTypeDef *config);

/**
 * @brief Software-reset the STMPE811
 * @param hts Touchscreen handle
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Reset(TS_HandleTypeDef *hts);

/**
 * @brief Configuration the driver applies when none is supplied
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
TS_StatusTypeDef TS_RegisterCallbacks(TS_HandleTypeDef *hts,
                                      void (*touch_callback)(void),
                                      void (*release_callback)(void),
                                      void (*gesture_callback)(TS_GestureTypeDef));

/**
 * @brief The handle registered by the most recent TS_Init()
 * @details The touch interrupt has no way to be told which handle to use, so it
 *          resolves it through here.
 * @return TS_HandleTypeDef* Registered handle, or NULL when none is active
 */
TS_HandleTypeDef* TS_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* TS_CORE_H */
