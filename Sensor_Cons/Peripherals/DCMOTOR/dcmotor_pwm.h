/**
 ******************************************************************************
 * @file    dcmotor_pwm.h
 * @brief   DC motor H-bridge transport - internal to the DCMOTOR driver
 * @details Owns the enable pin's alternate function, the PWM time base and the
 *          two direction outputs. Not part of dcmotor.h.
 ******************************************************************************
 */

#ifndef DCMOTOR_PWM_H
#define DCMOTOR_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dcmotor_types.h"

/**
 * @brief   Bring up the enable AF pin, the direction outputs and the PWM base.
 * @param   hmotor Handle with pins and config populated.
 * @retval  DCMOTOR_OK or DCMOTOR_INVALID_PARAM / DCMOTOR_ERROR
 * @note    Leaves the bridge released, so the motor never twitches on init.
 */
DCMOTOR_StatusTypeDef DCMOTOR_PWM_Init(DCMOTOR_Handle_t *hmotor);

/**
 * @brief   Stop PWM and release the enable and direction pins.
 * @param   pins Bridge pin assignment to release.
 */
void DCMOTOR_PWM_DeInit(const DCMOTOR_Pins_t *pins);

/**
 * @brief   Drive the enable pin at @p duty of the configured resolution.
 * @param   pins Bridge pin assignment.
 * @param   duty Compare value in 0..speedSteps.
 */
void DCMOTOR_PWM_SetDuty(const DCMOTOR_Pins_t *pins, uint32_t duty);

/**
 * @brief   Put the two direction inputs in the pattern for @p direction.
 * @param   pins      Bridge pin assignment.
 * @param   direction Requested rotation direction.
 */
void DCMOTOR_PWM_ApplyDirection(const DCMOTOR_Pins_t *pins, DCMOTOR_Direction_t direction);

/**
 * @brief   Put the two direction inputs in the pattern for @p stopMode.
 * @param   pins     Bridge pin assignment.
 * @param   stopMode Coast (both low) or brake (both high).
 */
void DCMOTOR_PWM_ApplyStop(const DCMOTOR_Pins_t *pins, DCMOTOR_StopMode_t stopMode);

#ifdef __cplusplus
}
#endif

#endif /* DCMOTOR_PWM_H */
