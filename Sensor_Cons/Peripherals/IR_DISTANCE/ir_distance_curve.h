/**
 ******************************************************************************
 * @file    ir_distance_curve.h
 * @brief   Calibration curves and piecewise linear interpolation
 ******************************************************************************
 */

#ifndef IR_DISTANCE_CURVE_H
#define IR_DISTANCE_CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_distance_types.h"

/**
 * @brief   Get the factory curve for a sensor type.
 * @retval  NULL for IR_DISTANCE_CUSTOM and for unknown types.
 */
const IR_DISTANCE_CustomCurve_t *
IR_DISTANCE_CURVE_GetPredefined(IR_DISTANCE_SensorType_t sensorType);

/**
 * @brief   Check that a curve is usable for interpolation.
 * @details Point count in range and both orderings strictly monotonic.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_Validate(const IR_DISTANCE_CustomCurve_t *curve);

/**
 * @brief   Interpolate a distance from a raw ADC reading.
 * @retval  IR_DISTANCE_OUT_OF_RANGE when the reading falls outside the curve.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_AdcToDistance(const IR_DISTANCE_CustomCurve_t *curve,
                                                          uint16_t adcValue, uint16_t *distance);

/**
 * @brief   Interpolate the expected ADC reading for a distance.
 * @retval  IR_DISTANCE_OUT_OF_RANGE when the distance falls outside the curve.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_DistanceToAdc(const IR_DISTANCE_CustomCurve_t *curve,
                                                          uint16_t distance, uint16_t *adcValue);

/**
 * @brief   Insert a point, keeping the curve sorted by distance.
 * @details An existing point at the same distance is replaced rather than
 *          duplicated, so repeated calibration at one spot cannot fill the curve.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_Insert(IR_DISTANCE_CustomCurve_t *curve,
                                                   uint16_t distance, uint16_t adcValue);

#ifdef __cplusplus
}
#endif

#endif /* IR_DISTANCE_CURVE_H */
