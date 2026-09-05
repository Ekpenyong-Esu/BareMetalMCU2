/**
 ******************************************************************************
 * @file    dcmotor_convert.c
 * @brief   Speed percent <-> duty conversion and range validation
 ******************************************************************************
 */

#include "dcmotor_convert.h"

/**
 * @brief Compare value for a speed percentage, clamped to the resolution.
 */
uint32_t DCMOTOR_PercentToDuty(uint8_t percent, uint16_t speedSteps) {
    if (percent >= DCMOTOR_MAX_SPEED_PERCENT) {
        return speedSteps;
    }

    /* Multiplying first keeps the ratio: percent / 100 truncates to zero. */
    return ((uint32_t)percent * speedSteps) / DCMOTOR_MAX_SPEED_PERCENT;
}

/**
 * @brief Speed percentage for a compare value, clamped to 0..100.
 */
uint8_t DCMOTOR_DutyToPercent(uint32_t duty, uint16_t speedSteps) {
    if (speedSteps == 0U) {
        return DCMOTOR_MIN_SPEED_PERCENT;
    }

    if (duty >= speedSteps) {
        return DCMOTOR_MAX_SPEED_PERCENT;
    }

    return (uint8_t)((duty * DCMOTOR_MAX_SPEED_PERCENT) / speedSteps);
}

/**
 * @brief Check a speed against the 0..100 percent range.
 */
bool DCMOTOR_IsValidSpeed(uint8_t percent) {
    /* percent is unsigned, so the DCMOTOR_MIN_SPEED_PERCENT (0) bound is implicit. */
    return (percent <= DCMOTOR_MAX_SPEED_PERCENT);
}

/**
 * @brief Reject a config whose carrier or resolution is outside the driver limits.
 */
DCMOTOR_StatusTypeDef DCMOTOR_ValidateConfig(const DCMOTOR_Config_t *config) {
    if (config == NULL) {
        return DCMOTOR_INVALID_PARAM;
    }

    if (config->pwmFrequencyHz < DCMOTOR_MIN_PWM_FREQUENCY_HZ ||
        config->pwmFrequencyHz > DCMOTOR_MAX_PWM_FREQUENCY_HZ) {
        return DCMOTOR_INVALID_PARAM;
    }

    if (config->speedSteps < DCMOTOR_MIN_SPEED_STEPS) {
        return DCMOTOR_INVALID_PARAM;
    }

    if (config->stopMode != DCMOTOR_STOP_COAST && config->stopMode != DCMOTOR_STOP_BRAKE) {
        return DCMOTOR_INVALID_PARAM;
    }

    return DCMOTOR_OK;
}
