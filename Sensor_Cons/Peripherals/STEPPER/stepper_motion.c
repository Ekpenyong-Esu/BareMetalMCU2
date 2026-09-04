/**
 ******************************************************************************
 * @file    stepper_motion.c
 * @brief   Movement commands and motor state
 ******************************************************************************
 */

#include "stepper_motion.h"
#include "stepper_sequence.h"
#include "stepper_gpio.h"
#include "stepper_timing.h"
#include "stepper_convert.h"

/**
 * @brief  Advance one position in the sequence, then energise the coils for it.
 * @note   Advancing first matters: applying the current entry would re-assert the
 *         pattern the coils already hold and lose a step per move.
 */
static void STEPPER_StepOnce(STEPPER_Handle_t *hstep) {
    uint8_t length = STEPPER_SEQ_GetLength(hstep->config.stepMode);

    if (hstep->direction == STEPPER_DIR_CW) {
        hstep->currentStep = (uint8_t)((hstep->currentStep + 1U) % length);
        hstep->currentPosition++;
    }
    else {
        hstep->currentStep = (uint8_t)((hstep->currentStep + length - 1U) % length);
        hstep->currentPosition--;
    }

    STEPPER_GPIO_ApplyPattern(&hstep->pins,
                              STEPPER_SEQ_GetPattern(hstep->config.stepMode, hstep->currentStep));
}

STEPPER_StatusTypeDef STEPPER_MoveSteps(STEPPER_Handle_t *hstep, uint32_t steps,
                                        STEPPER_Direction_t direction, uint16_t speed) {
    STEPPER_CHECK_HANDLE(hstep);

    if (hstep->isRunning) {
        return STEPPER_BUSY;
    }

    if (direction != STEPPER_DIR_CW && direction != STEPPER_DIR_CCW) {
        return STEPPER_INVALID_PARAM;
    }

    /* The per-motor ceiling is the one that protects the hardware; the global
       constant only bounds the conversion. */
    if (speed < STEPPER_MIN_SPEED_RPM || speed > hstep->config.maxSpeedRPM) {
        return STEPPER_INVALID_PARAM;
    }

    if (steps == 0U) {
        return STEPPER_OK;
    }

    hstep->direction = direction;
    hstep->stepDelay = STEPPER_RPMToDelay(speed, hstep->config.stepsPerRevolution);
    hstep->isRunning = true;

    for (uint32_t i = 0U; i < steps; i++) {
        STEPPER_StepOnce(hstep);
        STEPPER_TIMING_DelayMicroseconds(hstep->htim, hstep->stepDelay);
    }

    hstep->isRunning = false;

    return STEPPER_OK;
}

STEPPER_StatusTypeDef STEPPER_MoveToPosition(STEPPER_Handle_t *hstep, int32_t position,
                                             uint16_t speed) {
    int64_t delta = 0;
    STEPPER_Direction_t direction = STEPPER_DIR_CW;

    STEPPER_CHECK_HANDLE(hstep);

    /* 64-bit: the difference of two int32_t positions does not fit in an int32_t. */
    delta = (int64_t)position - (int64_t)hstep->currentPosition;

    if (delta == 0) {
        return STEPPER_OK;
    }

    direction = (delta > 0) ? STEPPER_DIR_CW : STEPPER_DIR_CCW;

    if (delta < 0) {
        delta = -delta;
    }

    return STEPPER_MoveSteps(hstep, (uint32_t)delta, direction, speed);
}

STEPPER_StatusTypeDef STEPPER_Stop(STEPPER_Handle_t *hstep) {
    STEPPER_CHECK_HANDLE(hstep);

    hstep->isRunning = false;
    STEPPER_GPIO_ReleaseCoils(&hstep->pins);

    return STEPPER_OK;
}

bool STEPPER_IsRunning(const STEPPER_Handle_t *hstep) {
    return (hstep != NULL) && hstep->isRunning;
}

int32_t STEPPER_GetPosition(const STEPPER_Handle_t *hstep) {
    return (hstep != NULL) ? hstep->currentPosition : 0;
}

STEPPER_StatusTypeDef STEPPER_SetPosition(STEPPER_Handle_t *hstep, int32_t position) {
    STEPPER_CHECK_HANDLE(hstep);

    if (hstep->isRunning) {
        return STEPPER_BUSY;
    }

    hstep->currentPosition = position;

    return STEPPER_OK;
}

STEPPER_StatusTypeDef STEPPER_GetStatus(const STEPPER_Handle_t *hstep) {
    STEPPER_CHECK_HANDLE(hstep);

    return hstep->isRunning ? STEPPER_BUSY : STEPPER_OK;
}
