/**
 ******************************************************************************
 * @file    dcmotor_control.c
 * @brief   DC motor drive commands
 ******************************************************************************
 */

#include "dcmotor_control.h"
#include "dcmotor_convert.h"
#include "dcmotor_pwm.h"

/* Releasing the bridge as well as zeroing the duty matters: an enable pin at 0 %
   already idles the motor, but leaving IN1/IN2 asserted keeps the H-bridge in a
   driven state that a later duty change would act on immediately. */
static DCMOTOR_StatusTypeDef DCMOTOR_Release(DCMOTOR_Handle_t *hmotor,
                                             DCMOTOR_StopMode_t stopMode) {
    DCMOTOR_PWM_SetDuty(&hmotor->pins, 0U);
    DCMOTOR_PWM_ApplyStop(&hmotor->pins, stopMode);

    hmotor->speedPercent = DCMOTOR_MIN_SPEED_PERCENT;
    hmotor->isRunning = false;

    return DCMOTOR_OK;
}

/**
 * @brief Set direction and speed in one command.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Drive(DCMOTOR_Handle_t *hmotor, DCMOTOR_Direction_t direction,
                                    uint8_t percent) {
    DCMOTOR_CHECK_HANDLE(hmotor);

    if (!DCMOTOR_IsValidSpeed(percent)) {
        return DCMOTOR_OUT_OF_RANGE;
    }

    hmotor->direction = direction;

    if (percent == DCMOTOR_MIN_SPEED_PERCENT) {
        return DCMOTOR_Release(hmotor, hmotor->config.stopMode);
    }

    /* Direction first: the bridge must already point the right way before any
       current flows, or the motor kicks the wrong way for one PWM period. */
    DCMOTOR_PWM_ApplyDirection(&hmotor->pins, direction);
    DCMOTOR_PWM_SetDuty(&hmotor->pins, DCMOTOR_PercentToDuty(percent, hmotor->config.speedSteps));

    hmotor->speedPercent = percent;
    hmotor->isRunning = true;

    return DCMOTOR_OK;
}

/**
 * @brief Drive forward at the given speed.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Forward(DCMOTOR_Handle_t *hmotor, uint8_t percent) {
    return DCMOTOR_Drive(hmotor, DCMOTOR_DIR_FORWARD, percent);
}

/**
 * @brief Drive in reverse at the given speed.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Reverse(DCMOTOR_Handle_t *hmotor, uint8_t percent) {
    return DCMOTOR_Drive(hmotor, DCMOTOR_DIR_REVERSE, percent);
}

/**
 * @brief Change speed without changing direction.
 */
DCMOTOR_StatusTypeDef DCMOTOR_SetSpeed(DCMOTOR_Handle_t *hmotor, uint8_t percent) {
    DCMOTOR_CHECK_HANDLE(hmotor);

    return DCMOTOR_Drive(hmotor, hmotor->direction, percent);
}

/**
 * @brief Release the drive using the configured stop mode.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Stop(DCMOTOR_Handle_t *hmotor) {
    DCMOTOR_CHECK_HANDLE(hmotor);

    return DCMOTOR_Release(hmotor, hmotor->config.stopMode);
}

/**
 * @brief Short the windings so the motor resists motion.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Brake(DCMOTOR_Handle_t *hmotor) {
    DCMOTOR_CHECK_HANDLE(hmotor);

    return DCMOTOR_Release(hmotor, DCMOTOR_STOP_BRAKE);
}

/**
 * @brief Open the bridge so the motor freewheels.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Coast(DCMOTOR_Handle_t *hmotor) {
    DCMOTOR_CHECK_HANDLE(hmotor);

    return DCMOTOR_Release(hmotor, DCMOTOR_STOP_COAST);
}

/**
 * @brief Speed last commanded (0 if handle is NULL).
 */
uint8_t DCMOTOR_GetSpeed(const DCMOTOR_Handle_t *hmotor) {
    return (hmotor != NULL) ? hmotor->speedPercent : (uint8_t)DCMOTOR_MIN_SPEED_PERCENT;
}

/**
 * @brief Direction last commanded (forward if handle is NULL).
 */
DCMOTOR_Direction_t DCMOTOR_GetDirection(const DCMOTOR_Handle_t *hmotor) {
    return (hmotor != NULL) ? hmotor->direction : DCMOTOR_DIR_FORWARD;
}

/**
 * @brief Whether a non-zero speed is currently applied.
 */
bool DCMOTOR_IsRunning(const DCMOTOR_Handle_t *hmotor) {
    if (hmotor == NULL) {
        return false;
    }

    return hmotor->isRunning;
}
