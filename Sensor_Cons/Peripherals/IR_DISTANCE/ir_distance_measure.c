/**
  ******************************************************************************
  * @file    ir_distance_measure.c
  * @brief   IR distance sensor measurement and conversion
  ******************************************************************************
  */

#include "ir_distance_measure.h"
#include "ir_distance_adc.h"
#include "ir_distance_curve.h"
#include "log.h"

IR_DISTANCE_StatusTypeDef IR_DISTANCE_Measure(IR_DISTANCE_Handle_t *hird, uint16_t *distance)
{
    uint32_t start;
    uint32_t adcSum = 0;
    uint16_t validSamples = 0;
    uint16_t measured;
    IR_DISTANCE_StatusTypeDef status = IR_DISTANCE_ERROR;

    IR_DISTANCE_CHECK_HANDLE(hird);

    hird->lastDistance = 0;

    start = HAL_GetTick();

    for (uint16_t i = 0; i < hird->config.averagingSamples; i++) {
        uint16_t sample;

        if (HAL_GetTick() - start > hird->config.measurementTimeout) {
            log_warning("IR_DISTANCE: measurement timed out after %u samples", validSamples);
            if (validSamples == 0) {
                return IR_DISTANCE_TIMEOUT;
            }
            break;
        }

        status = IR_DISTANCE_ADC_Read(hird->hadc, hird->channel, &sample);
        if (status == IR_DISTANCE_OK) {
            adcSum += sample;
            validSamples++;
        }

        HAL_Delay(IR_DISTANCE_SAMPLE_INTERVAL_MS);
    }

    if (validSamples == 0) {
        log_warning("IR_DISTANCE: no valid ADC samples obtained");
        return status;
    }

    hird->lastAdcValue = (uint16_t)(adcSum / validSamples);

    status = IR_DISTANCE_CURVE_AdcToDistance(&hird->customCurve, hird->lastAdcValue, &measured);
    if (status != IR_DISTANCE_OK) {
        log_warning("IR_DISTANCE: ADC %u falls outside the calibration curve",
                    hird->lastAdcValue);
        return status;
    }

    if (!IR_DISTANCE_IsValidDistance(hird, measured)) {
        log_warning("IR_DISTANCE: measured distance %u mm is out of the configured range",
                    measured);
        return IR_DISTANCE_OUT_OF_RANGE;
    }

    hird->lastDistance = measured;
    if (distance != NULL) {
        *distance = measured;
    }

    log_debug("IR_DISTANCE: ADC %u -> %u mm", hird->lastAdcValue, measured);

    return IR_DISTANCE_OK;
}

uint16_t IR_DISTANCE_MeasureDistance(IR_DISTANCE_Handle_t *hird)
{
    uint16_t distance = 0;

    (void)IR_DISTANCE_Measure(hird, &distance);

    return distance;
}

uint16_t IR_DISTANCE_GetDistance(const IR_DISTANCE_Handle_t *hird)
{
    return (hird != NULL) ? hird->lastDistance : 0U;
}

uint16_t IR_DISTANCE_GetAdcValue(const IR_DISTANCE_Handle_t *hird)
{
    return (hird != NULL) ? hird->lastAdcValue : 0U;
}

uint16_t IR_DISTANCE_AdcToDistance(const IR_DISTANCE_Handle_t *hird, uint16_t adcValue)
{
    uint16_t distance = 0;

    if (hird == NULL) {
        return 0;
    }

    (void)IR_DISTANCE_CURVE_AdcToDistance(&hird->customCurve, adcValue, &distance);

    return distance;
}

uint16_t IR_DISTANCE_DistanceToAdc(const IR_DISTANCE_Handle_t *hird, uint16_t distance)
{
    uint16_t adcValue = 0;

    if (hird == NULL) {
        return 0;
    }

    (void)IR_DISTANCE_CURVE_DistanceToAdc(&hird->customCurve, distance, &adcValue);

    return adcValue;
}

bool IR_DISTANCE_IsValidDistance(const IR_DISTANCE_Handle_t *hird, uint16_t distance)
{
    if (hird == NULL) {
        return false;
    }

    return (distance >= hird->config.minDistance && distance <= hird->config.maxDistance);
}
