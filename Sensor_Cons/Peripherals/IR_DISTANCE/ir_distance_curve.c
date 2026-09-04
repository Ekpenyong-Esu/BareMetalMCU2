/**
 ******************************************************************************
 * @file    ir_distance_curve.c
 * @brief   Calibration curves and piecewise linear interpolation
 ******************************************************************************
 */

#include "ir_distance_curve.h"

/* Sharp datasheet response curves, sampled at 12-bit / 3.3 V. */
static const IR_DISTANCE_CustomCurve_t gp2y0a21yk_curve = {.numPoints = 8,
                                                           .points = {{100, 3580},
                                                                      {150, 2500},
                                                                      {200, 1900},
                                                                      {250, 1500},
                                                                      {300, 1250},
                                                                      {400, 950},
                                                                      {500, 780},
                                                                      {800, 480}}};

static const IR_DISTANCE_CustomCurve_t gp2y0a02yk_curve = {.numPoints = 8,
                                                           .points = {{200, 3200},
                                                                      {300, 2200},
                                                                      {400, 1600},
                                                                      {500, 1250},
                                                                      {600, 1050},
                                                                      {800, 800},
                                                                      {1000, 650},
                                                                      {1500, 450}}};

static const IR_DISTANCE_CustomCurve_t gp2y0a41sk_curve = {
    .numPoints = 6,
    .points = {{40, 3800}, {60, 3200}, {80, 2700}, {100, 2300}, {150, 1700}, {300, 900}}};

static const IR_DISTANCE_CustomCurve_t gp2y0a51sk_curve = {
    .numPoints = 6,
    .points = {{20, 3900}, {30, 3500}, {50, 2900}, {70, 2400}, {100, 1800}, {150, 1200}}};

/**
 * @brief   Linear interpolation on int32 so a descending axis stays exact.
 */
static uint16_t IR_DISTANCE_Interpolate(int32_t input, int32_t startIn, int32_t startOut,
                                        int32_t endIn, int32_t endOut) {
    int32_t result = 0;

    if (startIn == endIn) {
        return (uint16_t)startOut;
    }

    result = startOut + ((endOut - startOut) * (input - startIn)) / (endIn - startIn);

    if (result < 0) {
        return 0;
    }
    if (result > UINT16_MAX) {
        return UINT16_MAX;
    }

    return (uint16_t)result;
}

const IR_DISTANCE_CustomCurve_t *
IR_DISTANCE_CURVE_GetPredefined(IR_DISTANCE_SensorType_t sensorType) {
    switch (sensorType) {
        case IR_DISTANCE_GP2Y0A21YK:
            return &gp2y0a21yk_curve;
        case IR_DISTANCE_GP2Y0A02YK:
            return &gp2y0a02yk_curve;
        case IR_DISTANCE_GP2Y0A41SK:
            return &gp2y0a41sk_curve;
        case IR_DISTANCE_GP2Y0A51SK:
            return &gp2y0a51sk_curve;
        default:
            return NULL;
    }
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_Validate(const IR_DISTANCE_CustomCurve_t *curve) {
    if (curve == NULL || curve->numPoints < IR_DISTANCE_MIN_CURVE_POINTS ||
        curve->numPoints > IR_DISTANCE_MAX_CURVE_POINTS) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    for (uint8_t i = 1; i < curve->numPoints; i++) {
        if (curve->points[i].distance <= curve->points[i - 1].distance ||
            curve->points[i].adcValue >= curve->points[i - 1].adcValue) {
            return IR_DISTANCE_INVALID_PARAM;
        }
    }

    return IR_DISTANCE_OK;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_AdcToDistance(const IR_DISTANCE_CustomCurve_t *curve,
                                                          uint16_t adcValue, uint16_t *distance) {
    if (distance == NULL || IR_DISTANCE_CURVE_Validate(curve) != IR_DISTANCE_OK) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < curve->numPoints - 1U; i++) {
        uint16_t adcNear = curve->points[i].adcValue;
        uint16_t adcFar = curve->points[i + 1].adcValue;

        if (adcValue <= adcNear && adcValue >= adcFar) {
            *distance = IR_DISTANCE_Interpolate(adcValue, adcNear, curve->points[i].distance,
                                                adcFar, curve->points[i + 1].distance);
            return IR_DISTANCE_OK;
        }
    }

    *distance = 0;

    return IR_DISTANCE_OUT_OF_RANGE;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_DistanceToAdc(const IR_DISTANCE_CustomCurve_t *curve,
                                                          uint16_t distance, uint16_t *adcValue) {
    if (adcValue == NULL || IR_DISTANCE_CURVE_Validate(curve) != IR_DISTANCE_OK) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < curve->numPoints - 1U; i++) {
        uint16_t distNear = curve->points[i].distance;
        uint16_t distFar = curve->points[i + 1].distance;

        if (distance >= distNear && distance <= distFar) {
            *adcValue = IR_DISTANCE_Interpolate(distance, distNear, curve->points[i].adcValue,
                                                distFar, curve->points[i + 1].adcValue);
            return IR_DISTANCE_OK;
        }
    }

    *adcValue = 0;

    return IR_DISTANCE_OUT_OF_RANGE;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_CURVE_Insert(IR_DISTANCE_CustomCurve_t *curve,
                                                   uint16_t distance, uint16_t adcValue) {
    uint8_t slot = 0;

    if (curve == NULL || curve->numPoints > IR_DISTANCE_MAX_CURVE_POINTS) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    for (slot = 0; slot < curve->numPoints; slot++) {
        if (curve->points[slot].distance >= distance) {
            break;
        }
    }

    if (slot < curve->numPoints && curve->points[slot].distance == distance) {
        curve->points[slot].adcValue = adcValue;
        return IR_DISTANCE_OK;
    }

    if (curve->numPoints >= IR_DISTANCE_MAX_CURVE_POINTS) {
        return IR_DISTANCE_ERROR;
    }

    for (uint8_t i = curve->numPoints; i > slot; i--) {
        curve->points[i] = curve->points[i - 1];
    }

    curve->points[slot].distance = distance;
    curve->points[slot].adcValue = adcValue;
    curve->numPoints++;

    return IR_DISTANCE_OK;
}
