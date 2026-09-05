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
 * @param   hservo   Uninitialised handle to populate (zeroed on entry).
 * @param   htim     Timer handle; caller must set Instance before call.
 * @param   channel  TIM_CHANNEL_1..4
 * @param   gpioPort GPIO port for PWM output.
 * @param   gpioPin  GPIO pin for PWM output.
 * @retval  SERVO_OK or SERVO_INVALID_PARAM / SERVO_ERROR
 */
SERVO_StatusTypeDef SERVO_Init(SERVO_Handle_t *hservo, TIM_HandleTypeDef *htim, uint32_t channel,
                               GPIO_TypeDef *gpioPort, uint16_t gpioPin);

/**
 * @brief   Stop PWM and mark handle uninitialised.
 * @param   hservo Initialised handle.
 * @retval  SERVO_OK or SERVO_NOT_INITIALIZED / SERVO_INVALID_PARAM
 */
SERVO_StatusTypeDef SERVO_DeInit(SERVO_Handle_t *hservo);

/**
 * @brief   Replace travel limits and move to the new default angle.
 * @param   hservo Initialised handle.
 * @param   config New limits; must pass SERVO_ValidateConfig.
 * @retval  SERVO_OK or validation / move error
 */
SERVO_StatusTypeDef SERVO_Config(SERVO_Handle_t *hservo, const SERVO_Config_t *config);

/**
 * @brief   Factory limits: 0-180°, 500-2500 us, default 90°.
 * @return  Default config.
 */
SERVO_Config_t SERVO_GetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CORE_H */
