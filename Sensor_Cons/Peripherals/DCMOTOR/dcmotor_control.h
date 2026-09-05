/**
 ******************************************************************************
 * @file    dcmotor_control.h
 * @brief   DC motor drive commands
 ******************************************************************************
 */

#ifndef DCMOTOR_CONTROL_H
#define DCMOTOR_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dcmotor_types.h"

/**
 * @brief   Set direction and speed in one command.
 * @param   hmotor    Initialised handle.
 * @param   direction Rotation direction.
 * @param   percent   Speed in percent (0..100); 0 stops the motor.
 * @retval  DCMOTOR_OK or DCMOTOR_OUT_OF_RANGE / handle error
 */
DCMOTOR_StatusTypeDef DCMOTOR_Drive(DCMOTOR_Handle_t *hmotor, DCMOTOR_Direction_t direction,
                                    uint8_t percent);

/**
 * @brief   Drive forward at the given speed.
 * @param   hmotor  Initialised handle.
 * @param   percent Speed in percent (0..100).
 * @retval  DCMOTOR_OK or DCMOTOR_OUT_OF_RANGE / handle error
 */
DCMOTOR_StatusTypeDef DCMOTOR_Forward(DCMOTOR_Handle_t *hmotor, uint8_t percent);

/**
 * @brief   Drive in reverse at the given speed.
 * @param   hmotor  Initialised handle.
 * @param   percent Speed in percent (0..100).
 * @retval  DCMOTOR_OK or DCMOTOR_OUT_OF_RANGE / handle error
 */
DCMOTOR_StatusTypeDef DCMOTOR_Reverse(DCMOTOR_Handle_t *hmotor, uint8_t percent);

/**
 * @brief   Change speed without changing direction.
 * @param   hmotor  Initialised handle.
 * @param   percent Speed in percent (0..100).
 * @retval  DCMOTOR_OK or DCMOTOR_OUT_OF_RANGE / handle error
 */
DCMOTOR_StatusTypeDef DCMOTOR_SetSpeed(DCMOTOR_Handle_t *hmotor, uint8_t percent);

/**
 * @brief   Release the drive using the configured stop mode.
 * @param   hmotor Initialised handle.
 * @retval  DCMOTOR_OK or handle error
 */
DCMOTOR_StatusTypeDef DCMOTOR_Stop(DCMOTOR_Handle_t *hmotor);

/**
 * @brief   Short the windings so the motor resists motion, ignoring stop mode.
 * @param   hmotor Initialised handle.
 * @retval  DCMOTOR_OK or handle error
 */
DCMOTOR_StatusTypeDef DCMOTOR_Brake(DCMOTOR_Handle_t *hmotor);

/**
 * @brief   Open the bridge so the motor freewheels, ignoring stop mode.
 * @param   hmotor Initialised handle.
 * @retval  DCMOTOR_OK or handle error
 */
DCMOTOR_StatusTypeDef DCMOTOR_Coast(DCMOTOR_Handle_t *hmotor);

/**
 * @brief   Speed last commanded.
 * @param   hmotor Handle or NULL (returns 0).
 * @return  Speed in percent.
 */
uint8_t DCMOTOR_GetSpeed(const DCMOTOR_Handle_t *hmotor);

/**
 * @brief   Direction last commanded.
 * @param   hmotor Handle or NULL (returns DCMOTOR_DIR_FORWARD).
 * @return  Rotation direction.
 */
DCMOTOR_Direction_t DCMOTOR_GetDirection(const DCMOTOR_Handle_t *hmotor);

/**
 * @brief   Whether a non-zero speed is currently applied.
 * @param   hmotor Handle or NULL (returns false).
 * @return  true while the motor is driven.
 */
bool DCMOTOR_IsRunning(const DCMOTOR_Handle_t *hmotor);

#ifdef __cplusplus
}
#endif

#endif /* DCMOTOR_CONTROL_H */
