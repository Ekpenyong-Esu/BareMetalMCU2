/**
  ******************************************************************************
  * @file    ir_distance_adc.h
  * @brief   ADC access for the IR distance sensor (internal)
  * @details Not part of the public ir_distance.h aggregator.
  ******************************************************************************
  */

#ifndef IR_DISTANCE_ADC_H
#define IR_DISTANCE_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_distance_types.h"

/**
 * @brief   Attach the sensor to an already initialised ADC handle.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_ADC_Init(ADC_HandleStruct *hadc, uint32_t channel);

/**
 * @brief   Single conversion.
 * @note    Reports failure through the return value, so a legitimate reading
 *          of 0 is not mistaken for an error.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_ADC_Read(ADC_HandleStruct *hadc, uint32_t channel,
                                               uint16_t *value);

#ifdef __cplusplus
}
#endif

#endif /* IR_DISTANCE_ADC_H */
