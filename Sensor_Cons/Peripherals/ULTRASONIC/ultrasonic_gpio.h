/**
 ******************************************************************************
 * @file    ultrasonic_gpio.h
 * @brief   Trigger and echo pin handling (internal)
 * @details Not part of the public ultrasonic.h aggregator.
 ******************************************************************************
 */

#ifndef ULTRASONIC_GPIO_H
#define ULTRASONIC_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ultrasonic_types.h"

/**
 * @brief   Configure the trigger pin as an output and the echo pin as the
 *          alternate function of the capture timer.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_GPIO_Init(const ULTRASONIC_Pins_t *pins,
                                              const TIM_TypeDef *instance);

void ULTRASONIC_GPIO_DeInit(const ULTRASONIC_Pins_t *pins);

void ULTRASONIC_GPIO_SetTrigger(const ULTRASONIC_Pins_t *pins, bool level);

#ifdef __cplusplus
}
#endif

#endif /* ULTRASONIC_GPIO_H */
