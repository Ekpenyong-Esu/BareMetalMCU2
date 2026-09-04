/**
 ******************************************************************************
 * @file    stepper_motion.h
 * @brief   Movement commands and motor state
 ******************************************************************************
 */

#ifndef STEPPER_MOTION_H
#define STEPPER_MOTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stepper_types.h"

/**
 * @brief   Step the motor a relative number of steps, blocking until done.
 * @param   speed Speed in RPM; must not exceed the configured maxSpeedRPM.
 */
STEPPER_StatusTypeDef STEPPER_MoveSteps(STEPPER_Handle_t *hstep, uint32_t steps,
                                        STEPPER_Direction_t direction, uint16_t speed);

/**
 * @brief   Step the motor to an absolute position, blocking until done.
 */
STEPPER_StatusTypeDef STEPPER_MoveToPosition(STEPPER_Handle_t *hstep, int32_t position,
                                             uint16_t speed);

/**
 * @brief   Clear the running flag and de-energise the coils.
 */
STEPPER_StatusTypeDef STEPPER_Stop(STEPPER_Handle_t *hstep);

bool STEPPER_IsRunning(const STEPPER_Handle_t *hstep);

/**
 * @brief   Current position in steps, signed relative to the last zero.
 */
int32_t STEPPER_GetPosition(const STEPPER_Handle_t *hstep);

/**
 * @brief   Redefine the current position, for homing or calibration.
 */
STEPPER_StatusTypeDef STEPPER_SetPosition(STEPPER_Handle_t *hstep, int32_t position);

/**
 * @brief   STEPPER_BUSY while moving, STEPPER_OK when idle and ready.
 */
STEPPER_StatusTypeDef STEPPER_GetStatus(const STEPPER_Handle_t *hstep);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_MOTION_H */
