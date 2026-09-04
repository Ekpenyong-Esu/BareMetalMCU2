/**
 ******************************************************************************
 * @file    stepper.c
 * @brief   Stepper motor lifecycle and configuration
 ******************************************************************************
 */

#include "stepper_core.h"
#include "stepper_motion.h"
#include "stepper_gpio.h"
#include "stepper_timing.h"
#include "stepper_convert.h"
#include "log.h"
#include <string.h>

#define STEPPER_DEFAULT_SPEED_RPM 60U
#define STEPPER_DEFAULT_MAX_RPM 500U

static STEPPER_StatusTypeDef STEPPER_ValidateConfig(const STEPPER_Config_t *config) {
    if (config->stepsPerRevolution == 0U) {
        return STEPPER_INVALID_PARAM;
    }

    if (config->maxSpeedRPM < STEPPER_MIN_SPEED_RPM ||
        config->maxSpeedRPM > STEPPER_MAX_SPEED_RPM) {
        return STEPPER_INVALID_PARAM;
    }

    switch (config->stepMode) {
        case STEPPER_MODE_FULL_STEP:
        case STEPPER_MODE_HALF_STEP:
        case STEPPER_MODE_WAVE_DRIVE:
            break;
        default:
            return STEPPER_INVALID_PARAM;
    }

    return STEPPER_OK;
}

STEPPER_StatusTypeDef STEPPER_Init(STEPPER_Handle_t *hstep, TIM_HandleTypeDef *htim,
                                   const STEPPER_Pins_t *pins) {
    STEPPER_StatusTypeDef status = STEPPER_OK;
    STEPPER_Config_t defaultConfig;

    if (hstep == NULL || htim == NULL || pins == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    memset(hstep, 0, sizeof(STEPPER_Handle_t));
    hstep->htim = htim;
    hstep->pins = *pins;

    status = STEPPER_GPIO_Init(&hstep->pins);
    if (status != STEPPER_OK) {
        return status;
    }

    status = STEPPER_TIMING_Init(htim);
    if (status != STEPPER_OK) {
        STEPPER_GPIO_DeInit(&hstep->pins);
        log_error("STEPPER: step time base initialization failed");
        return status;
    }

    /* Mark ready so STEPPER_Config, which guards on it, can proceed. */
    hstep->isInitialized = true;

    defaultConfig = STEPPER_GetDefaultConfig();
    status = STEPPER_Config(hstep, &defaultConfig);
    if (status != STEPPER_OK) {
        hstep->isInitialized = false;
        STEPPER_TIMING_DeInit(htim);
        STEPPER_GPIO_DeInit(&hstep->pins);
    }

    return status;
}

STEPPER_StatusTypeDef STEPPER_DeInit(STEPPER_Handle_t *hstep) {
    STEPPER_CHECK_HANDLE(hstep);

    (void)STEPPER_Stop(hstep);

    STEPPER_TIMING_DeInit(hstep->htim);
    STEPPER_GPIO_DeInit(&hstep->pins);

    hstep->isInitialized = false;

    return STEPPER_OK;
}

STEPPER_StatusTypeDef STEPPER_Config(STEPPER_Handle_t *hstep, const STEPPER_Config_t *config) {
    STEPPER_CHECK_HANDLE(hstep);

    if (config == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    if (hstep->isRunning) {
        return STEPPER_BUSY;
    }

    if (STEPPER_ValidateConfig(config) != STEPPER_OK) {
        return STEPPER_INVALID_PARAM;
    }

    hstep->config = *config;
    hstep->stepDelay = STEPPER_RPMToDelay(STEPPER_DEFAULT_SPEED_RPM, config->stepsPerRevolution);

    return STEPPER_OK;
}

STEPPER_Config_t STEPPER_GetDefaultConfig(void) {
    STEPPER_Config_t config = {.stepsPerRevolution = STEPPER_DEFAULT_STEPS_PER_REV,
                               .maxSpeedRPM = STEPPER_DEFAULT_MAX_RPM,
                               .stepMode = STEPPER_MODE_FULL_STEP};

    return config;
}

STEPPER_Pins_t STEPPER_GetDefaultPins(void) {
    STEPPER_Pins_t pins = {.port1 = GPIOE,
                           .pin1 = GPIO_PIN_4,
                           .port2 = GPIOE,
                           .pin2 = GPIO_PIN_5,
                           .port3 = GPIOE,
                           .pin3 = GPIO_PIN_6,
                           .port4 = GPIOB,
                           .pin4 = GPIO_PIN_6};

    return pins;
}
