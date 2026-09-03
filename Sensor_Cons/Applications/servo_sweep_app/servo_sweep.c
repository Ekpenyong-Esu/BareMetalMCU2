/**
 * @file servo_sweep.c
 * @brief Sweep pattern: which angles, in which order, how fast
 */

#include "servo_sweep.h"

#include <stddef.h>

#define SERVO_SWEEP_DEFAULT_STEP_DEG 5U
#define SERVO_SWEEP_DEFAULT_STEP_DELAY_MS 30U

ServoSweep_Config_t ServoSweep_GetDefaultConfig(void)
{
    ServoSweep_Config_t config = {
        .minAngle    = SERVO_MIN_ANGLE,
        .maxAngle    = SERVO_MAX_ANGLE,
        .stepDeg     = SERVO_SWEEP_DEFAULT_STEP_DEG,
        .stepDelayMs = SERVO_SWEEP_DEFAULT_STEP_DELAY_MS,
    };

    return config;
}

/** @brief Walk one direction, reporting each step that lands */
static SERVO_StatusTypeDef ServoSweep_Walk(SERVO_Handle_t *hservo,
                                           uint16_t fromDeg,
                                           uint16_t toDeg,
                                           uint16_t stepDeg,
                                           uint16_t stepDelayMs,
                                           ServoSweep_Progress_t progress)
{
    int32_t step = (int32_t)stepDeg;
    int32_t angle = (int32_t)fromDeg;
    int32_t end = (int32_t)toDeg;

    if (fromDeg > toDeg) {
        step = -step;
    }

    for (;;) {
        SERVO_StatusTypeDef status = SERVO_SetAngle(hservo, (uint16_t)angle);
        if (status != SERVO_OK) {
            return status;
        }
        if (progress != NULL) {
            progress((uint16_t)angle);
        }
        HAL_Delay(stepDelayMs);

        if (angle == end) {
            break;
        }
        angle += step;
        if ((step > 0 && angle > end) || (step < 0 && angle < end)) {
            angle = end;
        }
    }

    return SERVO_OK;
}

SERVO_StatusTypeDef ServoSweep_RunOnce(SERVO_Handle_t *hservo,
                                       const ServoSweep_Config_t *config,
                                       ServoSweep_Progress_t progress)
{
    if (hservo == NULL || config == NULL) {
        return SERVO_INVALID_PARAM;
    }
    if (config->stepDeg == 0U || config->minAngle > config->maxAngle) {
        return SERVO_INVALID_PARAM;
    }
    if (!SERVO_IsValidAngle(config->minAngle, &hservo->config) ||
        !SERVO_IsValidAngle(config->maxAngle, &hservo->config)) {
        return SERVO_OUT_OF_RANGE;
    }

    SERVO_StatusTypeDef status = SERVO_SetAngle(hservo, config->minAngle);
    if (status != SERVO_OK) {
        return status;
    }
    HAL_Delay(config->stepDelayMs);

    status = ServoSweep_Walk(hservo, config->minAngle, config->maxAngle,
                             config->stepDeg, config->stepDelayMs, progress);
    if (status != SERVO_OK) {
        return status;
    }

    return ServoSweep_Walk(hservo, config->maxAngle, config->minAngle,
                           config->stepDeg, config->stepDelayMs, progress);
}
