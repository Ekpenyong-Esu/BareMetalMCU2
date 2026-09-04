/**
 ******************************************************************************
 * @file    ir_distance_calibrate.h
 * @brief   IR distance sensor calibration
 ******************************************************************************
 */

#ifndef IR_DISTANCE_CALIBRATE_H
#define IR_DISTANCE_CALIBRATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_distance_types.h"

/**
 * @brief   Replace the active curve with a user supplied one.
 * @note    Rejected unless the curve is monotonic in both axes, which the
 *          interpolation relies on.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_SetCustomCurve(IR_DISTANCE_Handle_t *hird,
                                                     const IR_DISTANCE_CustomCurve_t *curve);

/**
 * @brief   Add or update one calibration point on the active curve.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_CalibratePoint(IR_DISTANCE_Handle_t *hird, uint16_t distance,
                                                     uint16_t adcValue);

/**
 * @brief   Restore the factory curve for the configured sensor type.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_ResetCalibration(IR_DISTANCE_Handle_t *hird);

#ifdef __cplusplus
}
#endif

#endif /* IR_DISTANCE_CALIBRATE_H */
