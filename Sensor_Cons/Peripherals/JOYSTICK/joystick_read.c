/**
 ******************************************************************************
 * @file    joystick_read.c
 * @brief   Joystick sampling - what the application calls in its loop
 ******************************************************************************
 */

#include "joystick_read.h"
#include "joystick_adc.h"
#include "joystick_convert.h"

/* Reading an axis is always the same two steps, so both axes and the combined
   read share this one helper. */
static JOYSTICK_StatusTypeDef JOYSTICK_ReadAxisPercent(const JOYSTICK_Handle_t *hjoy,
                                                       uint32_t channel, uint16_t center,
                                                       bool invert, int8_t *percent) {
    uint16_t raw = 0;

    JOYSTICK_StatusTypeDef status = JOYSTICK_ADC_ReadAxis(&hjoy->pins, channel, &raw);

    if (status != JOYSTICK_OK) {
        return status;
    }

    *percent = JOYSTICK_RawToPercent(raw, center, hjoy->config.rawMax,
                                     hjoy->config.deadzonePercent, invert);

    return JOYSTICK_OK;
}

/**
 * @brief Sample both axes and the button in one go.
 */
JOYSTICK_StatusTypeDef JOYSTICK_Read(const JOYSTICK_Handle_t *hjoy, JOYSTICK_Position_t *position) {
    JOYSTICK_CHECK_HANDLE(hjoy);

    if (position == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    JOYSTICK_StatusTypeDef status =
        JOYSTICK_ReadAxisPercent(hjoy, hjoy->pins.xChannel, hjoy->calibration.xCenter,
                                 hjoy->config.invertX, &position->xPercent);

    if (status != JOYSTICK_OK) {
        return status;
    }

    status = JOYSTICK_ReadAxisPercent(hjoy, hjoy->pins.yChannel, hjoy->calibration.yCenter,
                                      hjoy->config.invertY, &position->yPercent);

    if (status != JOYSTICK_OK) {
        return status;
    }

    position->buttonPressed = JOYSTICK_ADC_ReadButton(&hjoy->pins, hjoy->config.buttonActiveLow);

    return JOYSTICK_OK;
}

/**
 * @brief Sample the X axis alone.
 */
JOYSTICK_StatusTypeDef JOYSTICK_ReadX(const JOYSTICK_Handle_t *hjoy, int8_t *percent) {
    JOYSTICK_CHECK_HANDLE(hjoy);

    if (percent == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    return JOYSTICK_ReadAxisPercent(hjoy, hjoy->pins.xChannel, hjoy->calibration.xCenter,
                                    hjoy->config.invertX, percent);
}

/**
 * @brief Sample the Y axis alone.
 */
JOYSTICK_StatusTypeDef JOYSTICK_ReadY(const JOYSTICK_Handle_t *hjoy, int8_t *percent) {
    JOYSTICK_CHECK_HANDLE(hjoy);

    if (percent == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    return JOYSTICK_ReadAxisPercent(hjoy, hjoy->pins.yChannel, hjoy->calibration.yCenter,
                                    hjoy->config.invertY, percent);
}

/**
 * @brief Sample both axes and reduce them to one direction.
 */
JOYSTICK_StatusTypeDef JOYSTICK_ReadDirection(const JOYSTICK_Handle_t *hjoy,
                                              JOYSTICK_Direction_t *direction) {
    JOYSTICK_CHECK_HANDLE(hjoy);

    if (direction == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    JOYSTICK_Position_t position = {0};

    JOYSTICK_StatusTypeDef status = JOYSTICK_Read(hjoy, &position);

    if (status != JOYSTICK_OK) {
        return status;
    }

    *direction = JOYSTICK_PercentToDirection(position.xPercent, position.yPercent,
                                             hjoy->config.directionThresholdPercent);

    return JOYSTICK_OK;
}

/**
 * @brief Read the push switch.
 */
bool JOYSTICK_IsPressed(const JOYSTICK_Handle_t *hjoy) {

    if (hjoy == NULL || !hjoy->isInitialized) {
        return false;
    }

    return JOYSTICK_ADC_ReadButton(&hjoy->pins, hjoy->config.buttonActiveLow);
}

/**
 * @brief Test whether both axes are inside the deadzone.
 */
bool JOYSTICK_IsCentered(const JOYSTICK_Handle_t *hjoy) {
    JOYSTICK_Position_t position = {0};

    if (JOYSTICK_Read(hjoy, &position) != JOYSTICK_OK) {
        return false;
    }

    if (position.xPercent != 0) {
        return false;
    }

    return (position.yPercent == 0);
}
