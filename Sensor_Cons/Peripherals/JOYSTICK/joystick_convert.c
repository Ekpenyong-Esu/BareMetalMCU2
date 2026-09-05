/**
 ******************************************************************************
 * @file    joystick_convert.c
 * @brief   Joystick scaling maths - no hardware access
 ******************************************************************************
 */

#include "joystick_convert.h"

/* The two halves are scaled separately because the stick rarely rests at exactly
   mid-scale; sharing one span would cap travel on one side and overshoot on the
   other. */
static int32_t JOYSTICK_RawToSignedPercent(uint16_t raw, uint16_t center, uint16_t rawMax) {

    if (raw > center) {
        uint32_t span = (uint32_t)rawMax - (uint32_t)center;

        if (span == 0U) {
            return 0;
        }

        return (int32_t)(((uint32_t)(raw - center) * JOYSTICK_PERCENT_SCALE) / span);
    }

    if (raw < center) {
        uint32_t span = (uint32_t)center;

        if (span == 0U) {
            return 0;
        }

        return -(int32_t)(((uint32_t)(center - raw) * JOYSTICK_PERCENT_SCALE) / span);
    }

    return 0;
}

/* Travel outside the deadzone is stretched back over the full 0..100 range, so
   the actuator still reaches its limits instead of losing the dead band. */
static int32_t JOYSTICK_ApplyDeadzone(int32_t percent, uint8_t deadzonePercent) {
    int32_t magnitude = (percent < 0) ? -percent : percent;

    if (magnitude <= (int32_t)deadzonePercent) {
        return 0;
    }

    int32_t usable = (int32_t)JOYSTICK_PERCENT_SCALE - (int32_t)deadzonePercent;

    if (usable <= 0) {
        return 0;
    }

    int32_t scaled =
        ((magnitude - (int32_t)deadzonePercent) * (int32_t)JOYSTICK_PERCENT_SCALE) / usable;

    if (scaled > (int32_t)JOYSTICK_PERCENT_SCALE) {
        scaled = (int32_t)JOYSTICK_PERCENT_SCALE;
    }

    return (percent < 0) ? -scaled : scaled;
}

/**
 * @brief Turn a raw ADC count into signed deflection.
 */
int8_t JOYSTICK_RawToPercent(uint16_t raw, uint16_t center, uint16_t rawMax,
                             uint8_t deadzonePercent, bool invert) {
    int32_t percent = JOYSTICK_RawToSignedPercent(raw, center, rawMax);

    percent = JOYSTICK_ApplyDeadzone(percent, deadzonePercent);

    if (invert) {
        percent = -percent;
    }

    return (int8_t)percent;
}

/**
 * @brief Reduce two axes to a single direction.
 */
JOYSTICK_Direction_t JOYSTICK_PercentToDirection(int8_t xPercent, int8_t yPercent,
                                                 uint8_t thresholdPercent) {
    int32_t xMagnitude = (xPercent < 0) ? -(int32_t)xPercent : (int32_t)xPercent;
    int32_t yMagnitude = (yPercent < 0) ? -(int32_t)yPercent : (int32_t)yPercent;

    if (xMagnitude < (int32_t)thresholdPercent && yMagnitude < (int32_t)thresholdPercent) {
        return JOYSTICK_DIR_CENTER;
    }

    /* The larger deflection wins, so a diagonal push still yields one answer. */
    if (xMagnitude >= yMagnitude) {
        return (xPercent > 0) ? JOYSTICK_DIR_RIGHT : JOYSTICK_DIR_LEFT;
    }

    return (yPercent > 0) ? JOYSTICK_DIR_UP : JOYSTICK_DIR_DOWN;
}

/**
 * @brief Deflection without its sign.
 */
uint8_t JOYSTICK_Magnitude(int8_t percent) {

    if (percent < 0) {
        return (uint8_t)(-(int32_t)percent);
    }

    return (uint8_t)percent;
}

/**
 * @brief Check a configuration before it is stored.
 */
JOYSTICK_StatusTypeDef JOYSTICK_ValidateConfig(const JOYSTICK_Config_t *config) {

    if (config == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    if (config->rawMax < JOYSTICK_MIN_RAW_MAX) {
        return JOYSTICK_INVALID_PARAM;
    }

    if (config->deadzonePercent > JOYSTICK_MAX_DEADZONE_PERCENT) {
        return JOYSTICK_INVALID_PARAM;
    }

    if (config->directionThresholdPercent > JOYSTICK_PERCENT_SCALE) {
        return JOYSTICK_INVALID_PARAM;
    }

    return JOYSTICK_OK;
}
