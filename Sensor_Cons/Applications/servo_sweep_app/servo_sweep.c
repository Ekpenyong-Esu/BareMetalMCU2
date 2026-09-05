/**
 * @file servo_sweep.c
 * @brief Sweep pattern: which angles, in which order, how fast
 */

#include "servo_sweep.h"

#include "stm32f4xx_hal.h" /* HAL_Delay only */

#include <stddef.h>

#define SERVO_SWEEP_DEFAULT_STEP_DEG 5U
#define SERVO_SWEEP_DEFAULT_STEP_DELAY_MS 30U

ServoSweep_Config_t ServoSweep_GetDefaultConfig(const SERVO_Handle_t *hservo) {

    ServoSweep_Config_t config = {
        .minAngle = SERVO_MIN_ANGLE,
        .maxAngle = SERVO_MAX_ANGLE,
        .stepDeg = SERVO_SWEEP_DEFAULT_STEP_DEG,
        .stepDelayMs = SERVO_SWEEP_DEFAULT_STEP_DELAY_MS,
    };

    if (hservo != NULL) {
        config.minAngle = hservo->config.minAngle;
        config.maxAngle = hservo->config.maxAngle;
    }

    return config;
}

/** @brief Command one angle, report it, and let the horn settle */
static SERVO_StatusTypeDef ServoSweep_Step(SERVO_Handle_t *hservo, uint16_t angleDeg,
                                           uint16_t settleMs, ServoSweep_Progress_t progress) {

    SERVO_StatusTypeDef status = SERVO_SetAngle(hservo, angleDeg);
    if (status != SERVO_OK) {
        return status;
    }

    if (progress != NULL) {
        progress(angleDeg);
    }
    HAL_Delay(settleMs);

    return SERVO_OK;
}

/** @brief The angle after `angle` on the way to `end`, never overshooting */
static uint16_t ServoSweep_NextAngle(uint16_t angle, uint16_t end, uint16_t stepDeg) {

    if (angle < end) {
        return (end - angle > stepDeg) ? (uint16_t)(angle + stepDeg) : end;
    }
    return (angle - end > stepDeg) ? (uint16_t)(angle - stepDeg) : end;
}

/**
 * @brief Walk from `fromDeg` to `toDeg`, commanding every angle after the
 *        starting one. The caller has already put the servo at `fromDeg`.
 */
static SERVO_StatusTypeDef ServoSweep_Walk(SERVO_Handle_t *hservo,
                                           const ServoSweep_Config_t *config, uint16_t fromDeg,
                                           uint16_t toDeg, ServoSweep_Progress_t progress) {
                                            
    uint16_t angle = fromDeg;

    while (angle != toDeg) {
        angle = ServoSweep_NextAngle(angle, toDeg, config->stepDeg);

        SERVO_StatusTypeDef status = ServoSweep_Step(hservo, angle, config->stepDelayMs, progress);
        if (status != SERVO_OK) {
            return status;
        }
    }

    return SERVO_OK;
}

SERVO_StatusTypeDef ServoSweep_RunOnce(SERVO_Handle_t *hservo, const ServoSweep_Config_t *config,
                                       ServoSweep_Progress_t progress) {
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

    /* Starting point, then each leg picks up from where the other ended. */
    SERVO_StatusTypeDef status =
        ServoSweep_Step(hservo, config->minAngle, config->stepDelayMs, progress);
    if (status != SERVO_OK) {
        return status;
    }

    status = ServoSweep_Walk(hservo, config, config->minAngle, config->maxAngle, progress);
    if (status != SERVO_OK) {
        return status;
    }

    return ServoSweep_Walk(hservo, config, config->maxAngle, config->minAngle, progress);
}
