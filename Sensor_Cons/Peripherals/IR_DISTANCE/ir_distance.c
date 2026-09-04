/**
 ******************************************************************************
 * @file    ir_distance.c
 * @brief   IR distance sensor lifecycle and configuration
 ******************************************************************************
 */

#include "ir_distance_core.h"
#include "ir_distance_adc.h"
#include "ir_distance_curve.h"
#include "log.h"
#include <string.h>

static IR_DISTANCE_StatusTypeDef IR_DISTANCE_ValidateConfig(const IR_DISTANCE_Config_t *config) {
    if (config == NULL || config->sensorType > IR_DISTANCE_CUSTOM) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    if (config->minDistance >= config->maxDistance) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    if (config->averagingSamples == 0U ||
        config->averagingSamples > IR_DISTANCE_MAX_AVERAGING_SAMPLES) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    if (config->measurementTimeout == 0U) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    return IR_DISTANCE_OK;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_Init(IR_DISTANCE_Handle_t *hird, ADC_HandleStruct *hadc,
                                           uint32_t channel, IR_DISTANCE_SensorType_t sensorType) {
    IR_DISTANCE_Config_t defaultConfig;
    const IR_DISTANCE_CustomCurve_t *curve = NULL;
    IR_DISTANCE_StatusTypeDef status = IR_DISTANCE_OK;

    if (hird == NULL || hadc == NULL || sensorType > IR_DISTANCE_CUSTOM) {
        log_error("IR_DISTANCE: Invalid parameters provided to IR_DISTANCE_Init");
        return IR_DISTANCE_INVALID_PARAM;
    }

    log_debug("IR_DISTANCE: initializing on ADC channel %lu, sensor type %d",
              (unsigned long)channel, (int)sensorType);

    memset(hird, 0, sizeof(IR_DISTANCE_Handle_t));
    hird->hadc = hadc;
    hird->channel = channel;

    defaultConfig = IR_DISTANCE_GetDefaultConfig(sensorType);
    status = IR_DISTANCE_ValidateConfig(&defaultConfig);
    if (status != IR_DISTANCE_OK) {
        return status;
    }
    hird->config = defaultConfig;

    /* IR_DISTANCE_CUSTOM has no factory curve; the caller supplies one through
       IR_DISTANCE_SetCustomCurve before the first measurement. */
    curve = IR_DISTANCE_CURVE_GetPredefined(sensorType);
    if (curve != NULL) {
        hird->customCurve = *curve;
    }

    status = IR_DISTANCE_ADC_Init(hadc, channel);
    if (status != IR_DISTANCE_OK) {
        return status;
    }

    hird->isInitialized = true;

    log_info("IR_DISTANCE: IR distance sensor initialized successfully");

    return IR_DISTANCE_OK;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_DeInit(IR_DISTANCE_Handle_t *hird) {
    IR_DISTANCE_CHECK_HANDLE(hird);

    hird->isInitialized = false;
    hird->lastAdcValue = 0;
    hird->lastDistance = 0;

    return IR_DISTANCE_OK;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_Config(IR_DISTANCE_Handle_t *hird,
                                             const IR_DISTANCE_Config_t *config) {
    IR_DISTANCE_StatusTypeDef status = IR_DISTANCE_OK;

    if (hird == NULL) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    status = IR_DISTANCE_ValidateConfig(config);
    if (status != IR_DISTANCE_OK) {
        return status;
    }

    hird->config = *config;

    return IR_DISTANCE_OK;
}

IR_DISTANCE_Config_t IR_DISTANCE_GetDefaultConfig(IR_DISTANCE_SensorType_t sensorType) {
    IR_DISTANCE_Config_t config = {.sensorType = sensorType,
                                   .averagingSamples = IR_DISTANCE_DEFAULT_AVERAGING_SAMPLES,
                                   .measurementTimeout = IR_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT};

    switch (sensorType) {
        case IR_DISTANCE_GP2Y0A21YK:
            config.minDistance = IR_DISTANCE_GP2Y0A21YK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A21YK_MAX;
            break;
        case IR_DISTANCE_GP2Y0A02YK:
            config.minDistance = IR_DISTANCE_GP2Y0A02YK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A02YK_MAX;
            break;
        case IR_DISTANCE_GP2Y0A41SK:
            config.minDistance = IR_DISTANCE_GP2Y0A41SK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A41SK_MAX;
            break;
        case IR_DISTANCE_GP2Y0A51SK:
            config.minDistance = IR_DISTANCE_GP2Y0A51SK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A51SK_MAX;
            break;
        default:
            config.sensorType = IR_DISTANCE_CUSTOM;
            config.minDistance = IR_DISTANCE_CUSTOM_MIN;
            config.maxDistance = IR_DISTANCE_CUSTOM_MAX;
            break;
    }

    return config;
}
