/**
 ******************************************************************************
 * @file    ir_distance_core.h
 * @brief   IR distance sensor lifecycle and configuration
 ******************************************************************************
 */

#ifndef IR_DISTANCE_CORE_H
#define IR_DISTANCE_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_distance_types.h"

/**
 * @brief   Initialize an IR distance sensor on an ADC channel.
 * @param   hadc Already initialized ADC handle.
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_Init(IR_DISTANCE_Handle_t *hird, ADC_HandleStruct *hadc,
                                           uint32_t channel, IR_DISTANCE_SensorType_t sensorType);

IR_DISTANCE_StatusTypeDef IR_DISTANCE_DeInit(IR_DISTANCE_Handle_t *hird);

IR_DISTANCE_StatusTypeDef IR_DISTANCE_Config(IR_DISTANCE_Handle_t *hird,
                                             const IR_DISTANCE_Config_t *config);

IR_DISTANCE_Config_t IR_DISTANCE_GetDefaultConfig(IR_DISTANCE_SensorType_t sensorType);

#ifdef __cplusplus
}
#endif

#endif /* IR_DISTANCE_CORE_H */
