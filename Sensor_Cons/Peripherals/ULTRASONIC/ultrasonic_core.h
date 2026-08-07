/**
  ******************************************************************************
  * @file    ultrasonic_core.h
  * @brief   Ultrasonic sensor lifecycle and configuration
  ******************************************************************************
  */

#ifndef ULTRASONIC_CORE_H
#define ULTRASONIC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ultrasonic_types.h"

/**
 * @brief   Initialize the sensor.
 * @note    Reprograms the caller's timer as a free running 1 MHz counter.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_Init(ULTRASONIC_Handle_t *hultra,
                                         TIM_HandleTypeDef *htim,
                                         uint32_t channel,
                                         const ULTRASONIC_Pins_t *pins);

ULTRASONIC_StatusTypeDef ULTRASONIC_DeInit(ULTRASONIC_Handle_t *hultra);

ULTRASONIC_StatusTypeDef ULTRASONIC_Config(ULTRASONIC_Handle_t *hultra,
                                           const ULTRASONIC_Config_t *config);

ULTRASONIC_Config_t ULTRASONIC_GetDefaultConfig(void);

ULTRASONIC_StatusTypeDef ULTRASONIC_SetTemperature(ULTRASONIC_Handle_t *hultra,
                                                   int8_t temperature);

#ifdef __cplusplus
}
#endif

#endif /* ULTRASONIC_CORE_H */
