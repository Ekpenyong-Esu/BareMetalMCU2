/**
  ******************************************************************************
  * @file    ultrasonic.c
  * @brief   Ultrasonic sensor lifecycle and configuration
  ******************************************************************************
  */

#include "ultrasonic_core.h"
#include "ultrasonic_capture.h"
#include "ultrasonic_gpio.h"
#include "log.h"
#include <string.h>

static ULTRASONIC_StatusTypeDef ULTRASONIC_ValidateConfig(const ULTRASONIC_Config_t *config)
{
    if (config == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    if (config->minDistance >= config->maxDistance ||
        config->maxDistance > ULTRASONIC_ABS_MAX_DISTANCE) {
        return ULTRASONIC_INVALID_PARAM;
    }

    if (config->temperature < ULTRASONIC_MIN_TEMPERATURE ||
        config->temperature > ULTRASONIC_MAX_TEMPERATURE) {
        return ULTRASONIC_INVALID_PARAM;
    }

    if (config->measurementTimeout == 0U) {
        return ULTRASONIC_INVALID_PARAM;
    }

    return ULTRASONIC_OK;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_Init(ULTRASONIC_Handle_t *hultra,
                                         TIM_HandleTypeDef *htim,
                                         uint32_t channel,
                                         const ULTRASONIC_Pins_t *pins)
{
    ULTRASONIC_Config_t defaultConfig;
    ULTRASONIC_StatusTypeDef status;

    if (hultra == NULL || htim == NULL || htim->Instance == NULL || pins == NULL ||
        pins->triggerPort == NULL || pins->echoPort == NULL) {
        log_error("ULTRASONIC: Invalid parameters provided to ULTRASONIC_Init");
        return ULTRASONIC_INVALID_PARAM;
    }

    log_debug("ULTRASONIC: initializing on capture channel %lu", (unsigned long)channel);

    memset(hultra, 0, sizeof(ULTRASONIC_Handle_t));
    hultra->htim = htim;
    hultra->channel = channel;
    hultra->pins = *pins;
    hultra->echoState = ULTRASONIC_ECHO_IDLE;
    hultra->measurementDone = true;

    defaultConfig = ULTRASONIC_GetDefaultConfig();
    status = ULTRASONIC_ValidateConfig(&defaultConfig);
    if (status != ULTRASONIC_OK) {
        return status;
    }
    hultra->config = defaultConfig;

    status = ULTRASONIC_GPIO_Init(&hultra->pins, htim->Instance);
    if (status != ULTRASONIC_OK) {
        return status;
    }

    status = ULTRASONIC_CAPTURE_Init(htim, channel);
    if (status != ULTRASONIC_OK) {
        ULTRASONIC_GPIO_DeInit(&hultra->pins);
        return status;
    }

    hultra->isInitialized = true;

    log_info("ULTRASONIC: Ultrasonic sensor initialized successfully");

    return ULTRASONIC_OK;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_DeInit(ULTRASONIC_Handle_t *hultra)
{
    ULTRASONIC_CHECK_HANDLE(hultra);

    (void)ULTRASONIC_CAPTURE_DeInit(hultra->htim, hultra->channel);
    ULTRASONIC_GPIO_DeInit(&hultra->pins);

    hultra->echoState = ULTRASONIC_ECHO_IDLE;
    hultra->measurementDone = true;
    hultra->isInitialized = false;

    return ULTRASONIC_OK;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_Config(ULTRASONIC_Handle_t *hultra,
                                           const ULTRASONIC_Config_t *config)
{
    ULTRASONIC_StatusTypeDef status;

    if (hultra == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    status = ULTRASONIC_ValidateConfig(config);
    if (status != ULTRASONIC_OK) {
        return status;
    }

    hultra->config = *config;

    return ULTRASONIC_OK;
}

ULTRASONIC_Config_t ULTRASONIC_GetDefaultConfig(void)
{
    ULTRASONIC_Config_t config = {
        .measurementTimeout = ULTRASONIC_DEFAULT_MEASUREMENT_TIMEOUT,
        .minDistance = ULTRASONIC_DEFAULT_MIN_DISTANCE,
        .maxDistance = ULTRASONIC_DEFAULT_MAX_DISTANCE,
        .temperature = ULTRASONIC_DEFAULT_TEMPERATURE
    };

    return config;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_SetTemperature(ULTRASONIC_Handle_t *hultra,
                                                   int8_t temperature)
{
    ULTRASONIC_CHECK_HANDLE(hultra);

    if (temperature < ULTRASONIC_MIN_TEMPERATURE || temperature > ULTRASONIC_MAX_TEMPERATURE) {
        return ULTRASONIC_INVALID_PARAM;
    }

    hultra->config.temperature = temperature;

    return ULTRASONIC_OK;
}
