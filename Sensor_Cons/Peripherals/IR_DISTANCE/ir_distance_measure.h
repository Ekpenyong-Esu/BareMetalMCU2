/**
 ******************************************************************************
 * @file    ir_distance_measure.h
 * @brief   IR distance sensor measurement and conversion
 ******************************************************************************
 */

#ifndef IR_DISTANCE_MEASURE_H
#define IR_DISTANCE_MEASURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_distance_types.h"

/**
 * @brief   Averaged measurement, reporting why it failed.
 * @param   distance Optional, receives the distance in mm.
 * @retval  IR_DISTANCE_OUT_OF_RANGE when the target is outside the sensor range.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_Measure(IR_DISTANCE_Handle_t *hird, uint16_t *distance);

/**
 * @brief   Averaged measurement.
 * @retval  Distance in mm, 0 on any failure.
 */
uint16_t IR_DISTANCE_MeasureDistance(IR_DISTANCE_Handle_t *hird);

/**
 * @brief   Last in-range distance in mm, 0 if the last measurement failed.
 */
uint16_t IR_DISTANCE_GetDistance(const IR_DISTANCE_Handle_t *hird);

/**
 * @brief   Last averaged raw ADC reading.
 */
uint16_t IR_DISTANCE_GetAdcValue(const IR_DISTANCE_Handle_t *hird);

/**
 * @brief   Convert a raw ADC reading to a distance, 0 if outside the curve.
 */
uint16_t IR_DISTANCE_AdcToDistance(const IR_DISTANCE_Handle_t *hird, uint16_t adcValue);

/**
 * @brief   Convert a distance to the expected ADC reading, 0 if outside the curve.
 */
uint16_t IR_DISTANCE_DistanceToAdc(const IR_DISTANCE_Handle_t *hird, uint16_t distance);

bool IR_DISTANCE_IsValidDistance(const IR_DISTANCE_Handle_t *hird, uint16_t distance);

#ifdef __cplusplus
}
#endif

#endif /* IR_DISTANCE_MEASURE_H */
