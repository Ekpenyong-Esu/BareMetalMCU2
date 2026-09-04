/**
 ******************************************************************************
 * @file    servo_pwm.h
 * @brief   Servo PWM transport - internal to the SERVO driver
 * @details Owns the GPIO alternate-function pin, the 50 Hz timer base and the
 *          microsecond-to-compare-value mapping. Not part of servo.h.
 ******************************************************************************
 */

#ifndef SERVO_PWM_H
#define SERVO_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "servo_types.h"

/**
 * @brief   Bring up the AF pin and program the timer for 50 Hz with 1 us steps.
 * @note    Fails if the timer has no output channels or its clock cannot yield
 *          an exact 1 MHz counter, because the pulse width is then not in us.
 */
SERVO_StatusTypeDef SERVO_PWM_Init(SERVO_Handle_t *hservo);

void SERVO_PWM_DeInit(SERVO_Handle_t *hservo);

/**
 * @brief   Drive the output at @p pulseWidthUs of the 20 ms frame.
 */
SERVO_StatusTypeDef SERVO_PWM_SetPulseWidth(SERVO_Handle_t *hservo, uint16_t pulseWidthUs);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_PWM_H */
