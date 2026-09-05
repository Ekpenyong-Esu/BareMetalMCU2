/**
 ******************************************************************************
 * @file    joystick.c
 * @brief   Joystick lifecycle - bring-up, configuration and calibration
 ******************************************************************************
 */

#include "joystick_core.h"
#include "joystick_adc.h"
#include "joystick_convert.h"
#include <string.h>

#define JOYSTICK_CENTER_DIVISOR 2U /*!< Mid-scale assumption before calibration */

/**
 * @brief Sensible starting configuration.
 */
JOYSTICK_Config_t JOYSTICK_GetDefaultConfig(void) {
    JOYSTICK_Config_t config = {
        .rawMax = (uint16_t)JOYSTICK_DEFAULT_RAW_MAX,
        .deadzonePercent = (uint8_t)JOYSTICK_DEFAULT_DEADZONE_PERCENT,
        .directionThresholdPercent = (uint8_t)JOYSTICK_DEFAULT_DIRECTION_THRESHOLD_PERCENT,
        .invertX = false,
        .invertY = false,
        .buttonActiveLow = true,
    };

    return config;
}

/**
 * @brief Prepare a joystick for reading.
 */
JOYSTICK_StatusTypeDef JOYSTICK_Init(JOYSTICK_Handle_t *hjoy, const JOYSTICK_Pins_t *pins) {

    if (hjoy == NULL || pins == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    memset(hjoy, 0, sizeof(*hjoy));

    hjoy->pins = *pins;
    hjoy->config = JOYSTICK_GetDefaultConfig();

    JOYSTICK_StatusTypeDef status = JOYSTICK_ADC_Init(&hjoy->pins, &hjoy->config);

    if (status != JOYSTICK_OK) {
        return status;
    }

    hjoy->calibration.xCenter = (uint16_t)(hjoy->config.rawMax / JOYSTICK_CENTER_DIVISOR);
    hjoy->calibration.yCenter = hjoy->calibration.xCenter;

    hjoy->isInitialized = true;

    return JOYSTICK_OK;
}

/**
 * @brief Release the joystick.
 */
JOYSTICK_StatusTypeDef JOYSTICK_DeInit(JOYSTICK_Handle_t *hjoy) {
    JOYSTICK_CHECK_HANDLE(hjoy);

    JOYSTICK_ADC_DeInit(&hjoy->pins);

    hjoy->isInitialized = false;

    return JOYSTICK_OK;
}

/**
 * @brief Replace the scaling and orientation settings.
 */
JOYSTICK_StatusTypeDef JOYSTICK_Config(JOYSTICK_Handle_t *hjoy, const JOYSTICK_Config_t *config) {
    JOYSTICK_CHECK_HANDLE(hjoy);

    JOYSTICK_StatusTypeDef status = JOYSTICK_ValidateConfig(config);

    if (status != JOYSTICK_OK) {
        return status;
    }

    /* Only the button pull depends on hardware state, so that is all that is redone. */
    status = JOYSTICK_ADC_Init(&hjoy->pins, config);

    if (status != JOYSTICK_OK) {
        return status;
    }

    hjoy->config = *config;

    return JOYSTICK_OK;
}

/**
 * @brief Record where the stick rests and treat that as zero.
 */
JOYSTICK_StatusTypeDef JOYSTICK_Calibrate(JOYSTICK_Handle_t *hjoy) {
    JOYSTICK_CHECK_HANDLE(hjoy);

    uint32_t xSum = 0;
    uint32_t ySum = 0;

    for (uint32_t sample = 0; sample < JOYSTICK_CALIBRATION_SAMPLES; sample++) {
        uint16_t xRaw = 0;
        uint16_t yRaw = 0;

        if (JOYSTICK_ADC_ReadAxis(&hjoy->pins, hjoy->pins.xChannel, &xRaw) != JOYSTICK_OK) {
            return JOYSTICK_ERROR;
        }

        if (JOYSTICK_ADC_ReadAxis(&hjoy->pins, hjoy->pins.yChannel, &yRaw) != JOYSTICK_OK) {
            return JOYSTICK_ERROR;
        }

        xSum += xRaw;
        ySum += yRaw;
    }

    hjoy->calibration.xCenter = (uint16_t)(xSum / JOYSTICK_CALIBRATION_SAMPLES);
    hjoy->calibration.yCenter = (uint16_t)(ySum / JOYSTICK_CALIBRATION_SAMPLES);

    return JOYSTICK_OK;
}
