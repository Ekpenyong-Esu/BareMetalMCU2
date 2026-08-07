/**
  ******************************************************************************
  * @file    servo_convert.c
  * @brief   Angle <-> pulse width conversion and range validation
  ******************************************************************************
  */

#include "servo_convert.h"

uint16_t SERVO_AngleToPulseWidth(uint16_t angle, const SERVO_Config_t *config)
{
    if (config == NULL) {
        return SERVO_DEFAULT_PULSE_WIDTH_US;
    }

    uint32_t angleRange = (uint32_t)config->maxAngle - config->minAngle;
    if (angleRange == 0U) {
        return config->minPulseWidth;
    }

    /* Clamping first keeps the subtraction below from going negative. */
    if (angle <= config->minAngle) {
        return config->minPulseWidth;
    }
    if (angle >= config->maxAngle) {
        return config->maxPulseWidth;
    }

    uint32_t pulseRange = (uint32_t)config->maxPulseWidth - config->minPulseWidth;
    uint32_t offset = ((uint32_t)(angle - config->minAngle) * pulseRange) / angleRange;

    return (uint16_t)(config->minPulseWidth + offset);
}

uint16_t SERVO_PulseWidthToAngle(uint16_t pulseWidth, const SERVO_Config_t *config)
{
    if (config == NULL) {
        return SERVO_DEFAULT_ANGLE;
    }

    uint32_t pulseRange = (uint32_t)config->maxPulseWidth - config->minPulseWidth;
    if (pulseRange == 0U) {
        return config->minAngle;
    }

    if (pulseWidth <= config->minPulseWidth) {
        return config->minAngle;
    }
    if (pulseWidth >= config->maxPulseWidth) {
        return config->maxAngle;
    }

    uint32_t angleRange = (uint32_t)config->maxAngle - config->minAngle;
    uint32_t offset = ((uint32_t)(pulseWidth - config->minPulseWidth) * angleRange) / pulseRange;

    return (uint16_t)(config->minAngle + offset);
}

bool SERVO_IsValidAngle(uint16_t angle, const SERVO_Config_t *config)
{
    if (config == NULL) {
        /* angle is unsigned, so the SERVO_MIN_ANGLE (0) bound is implicit. */
        return (angle <= SERVO_MAX_ANGLE);
    }

    return (angle >= config->minAngle && angle <= config->maxAngle);
}

bool SERVO_IsValidPulseWidth(uint16_t pulseWidth, const SERVO_Config_t *config)
{
    if (config == NULL) {
        return (pulseWidth >= SERVO_MIN_PULSE_WIDTH_US &&
                pulseWidth <= SERVO_MAX_PULSE_WIDTH_US);
    }

    return (pulseWidth >= config->minPulseWidth && pulseWidth <= config->maxPulseWidth);
}

SERVO_StatusTypeDef SERVO_ValidateConfig(const SERVO_Config_t *config)
{
    if (config == NULL) {
        return SERVO_INVALID_PARAM;
    }

    if (config->minAngle >= config->maxAngle) {
        return SERVO_INVALID_PARAM;
    }

    if (config->minPulseWidth >= config->maxPulseWidth) {
        return SERVO_INVALID_PARAM;
    }

    /* A pulse outside this window is not a servo command any hobby servo accepts. */
    if (config->minPulseWidth < SERVO_MIN_PULSE_WIDTH_US ||
        config->maxPulseWidth > SERVO_MAX_PULSE_WIDTH_US) {
        return SERVO_INVALID_PARAM;
    }

    if (config->defaultAngle < config->minAngle || config->defaultAngle > config->maxAngle) {
        return SERVO_INVALID_PARAM;
    }

    return SERVO_OK;
}
