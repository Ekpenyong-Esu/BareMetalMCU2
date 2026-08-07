/**
 * @file ts_touch.h
 * @brief Touch acquisition: reading, mapping and smoothing a touch point
 */

#ifndef TS_TOUCH_H
#define TS_TOUCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_types.h"

/**
 * @brief Copy out the touch data cached in the handle
 * @param hts Touchscreen handle
 * @param touch_data Destination
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_GetTouchData(TS_HandleTypeDef *hts, TS_TouchDataTypeDef *touch_data);

/**
 * @brief Read the current touch point in display coordinates
 * @param hts Touchscreen handle
 * @param xPos Destination X, set to 0 when nothing is touched
 * @param yPos Destination Y, set to 0 when nothing is touched
 * @return TS_StatusTypeDef TS_OK on a touch, TS_ERROR when the panel is idle
 */
TS_StatusTypeDef TS_GetSingleTouch(TS_HandleTypeDef *hts, uint16_t *xPos, uint16_t *yPos);

/**
 * @brief Read the current touch point, reporting an idle panel as a normal result
 * @param hts Touchscreen handle
 * @param x Destination X
 * @param y Destination Y
 * @param pressed Set to 1 while the panel is touched, 0 otherwise
 * @return TS_StatusTypeDef TS_OK even when nothing is touched
 */
TS_StatusTypeDef TS_GetTouchState(TS_HandleTypeDef *hts, uint16_t *x, uint16_t *y, uint8_t *pressed);

/**
 * @brief Report whether the controller FIFO holds any samples
 * @param hts Touchscreen handle
 * @return bool true when a touch is queued
 */
bool TS_IsTouched(TS_HandleTypeDef *hts);

/**
 * @brief Number of active touches recorded in the handle
 * @param hts Touchscreen handle
 * @return uint8_t Touch count
 */
uint8_t TS_GetTouchCount(TS_HandleTypeDef *hts);

/**
 * @brief Read the touch pressure
 * @param hts Touchscreen handle
 * @param pressure Destination, scaled to 0-255
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_GetPressure(TS_HandleTypeDef *hts, uint16_t *pressure);

#ifdef __cplusplus
}
#endif

#endif /* TS_TOUCH_H */
