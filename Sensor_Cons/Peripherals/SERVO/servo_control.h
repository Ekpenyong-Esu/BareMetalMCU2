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

/**
 * @brief   Move to angle; rejects out-of-range.
 * @param   hservo Initialised handle.
 * @param   angle  Target angle in degrees.
 * @retval  SERVO_OK or SERVO_OUT_OF_RANGE / handle error
 */
SERVO_StatusTypeDef SERVO_SetAngle(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Drive a pulse width directly; the reported angle follows it.
 * @param   hservo     Initialised handle.
 * @param   pulseWidth Pulse width in microseconds.
 * @retval  SERVO_OK or SERVO_INVALID_PARAM / handle error
 */
SERVO_StatusTypeDef SERVO_SetPulseWidth(SERVO_Handle_t *hservo, uint16_t pulseWidth);

/**
 * @brief   Last commanded angle.
 * @param   hservo Handle or NULL (returns 0).
 * @return  Angle in degrees.
 */
uint16_t SERVO_GetAngle(const SERVO_Handle_t *hservo);

/**
 * @brief   Last commanded pulse width.
 * @param   hservo Handle or NULL (returns 0).
 * @return  Pulse width in microseconds.
 */
uint16_t SERVO_GetPulseWidth(const SERVO_Handle_t *hservo);

/**
 * @brief   Move to configured minimum angle.
 * @param   hservo Initialised handle.
 * @retval  SERVO_OK or handle / range error
 */
SERVO_StatusTypeDef SERVO_MoveToMin(SERVO_Handle_t *hservo);

/**
 * @brief   Move to configured maximum angle.
 * @param   hservo Initialised handle.
 * @retval  SERVO_OK or handle / range error
 */
SERVO_StatusTypeDef SERVO_MoveToMax(SERVO_Handle_t *hservo);

/**
 * @brief   Move to midpoint of configured range.
 * @param   hservo Initialised handle.
 * @retval  SERVO_OK or handle / range error
 */
SERVO_StatusTypeDef SERVO_MoveToCenter(SERVO_Handle_t *hservo);

/**
 * @brief   Blocking sweep from min to max and back.
 * @param   hservo Initialised handle.
 * @param   speed  Delay between one-degree steps in milliseconds.
 * @retval  SERVO_OK or handle / range error
 */
SERVO_StatusTypeDef SERVO_Sweep(SERVO_Handle_t *hservo, uint16_t speed);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CONTROL_H */
