/**
  ******************************************************************************
  * @file    servo_control.h
  * @brief   Servo positioning commands
  ******************************************************************************
  */

#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "servo_types.h"

/* Moves to an angle within the servo's configured range. */
SERVO_StatusTypeDef SERVO_SetAngle(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Drive a pulse width directly; the reported angle follows it.
 */
SERVO_StatusTypeDef SERVO_SetPulseWidth(SERVO_Handle_t *hservo, uint16_t pulseWidth);

uint16_t SERVO_GetAngle(const SERVO_Handle_t *hservo);

uint16_t SERVO_GetPulseWidth(const SERVO_Handle_t *hservo);

/* Moves to the configured minimum-angle endpoint. */
SERVO_StatusTypeDef SERVO_MoveToMin(SERVO_Handle_t *hservo);

/* Moves to the configured maximum-angle endpoint. */
SERVO_StatusTypeDef SERVO_MoveToMax(SERVO_Handle_t *hservo);

/** @brief Midpoint of the configured range, not a fixed 90 degrees. */
SERVO_StatusTypeDef SERVO_MoveToCenter(SERVO_Handle_t *hservo);

/**
 * @brief   Blocking sweep from min to max and back.
 * @param   speed Delay between one-degree steps in milliseconds.
 */
SERVO_StatusTypeDef SERVO_Sweep(SERVO_Handle_t *hservo, uint16_t speed);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CONTROL_H */
