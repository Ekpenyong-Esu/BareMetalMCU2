/**
 ******************************************************************************
 * @file    servo_core.h
 * @brief   Servo lifecycle and configuration
 ******************************************************************************
 */

#ifndef SERVO_CORE_H
#define SERVO_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "servo_types.h"

/**
 * @brief   Bring up the PWM pin and timer and move to the default angle.
 * @param   htim     Timer handle; its Instance is programmed for 50 Hz.
 * @param   channel  TIM_CHANNEL_1..4
 */
SERVO_StatusTypeDef SERVO_Init(SERVO_Handle_t *hservo, TIM_HandleTypeDef *htim, uint32_t channel,
                               GPIO_TypeDef *gpioPort, uint16_t gpioPin);

SERVO_StatusTypeDef SERVO_DeInit(SERVO_Handle_t *hservo);

/**
 * @brief   Replace the travel limits and move to the new default angle.
 */
SERVO_StatusTypeDef SERVO_Config(SERVO_Handle_t *hservo, const SERVO_Config_t *config);

SERVO_Config_t SERVO_GetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CORE_H */
