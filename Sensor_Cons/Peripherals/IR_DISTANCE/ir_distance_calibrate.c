/**
  ******************************************************************************
  * @file    ir_distance_calibrate.c
  * @brief   IR distance sensor calibration
  ******************************************************************************
  */

#include "ir_distance_calibrate.h"
#include "ir_distance_curve.h"
#include "log.h"

IR_DISTANCE_StatusTypeDef IR_DISTANCE_SetCustomCurve(IR_DISTANCE_Handle_t *hird,
                                                     const IR_DISTANCE_CustomCurve_t *curve)
{
    IR_DISTANCE_StatusTypeDef status;

    if (hird == NULL) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    status = IR_DISTANCE_CURVE_Validate(curve);
    if (status != IR_DISTANCE_OK) {
        log_error("IR_DISTANCE: rejected calibration curve");
        return status;
    }

    hird->customCurve = *curve;
    hird->config.sensorType = IR_DISTANCE_CUSTOM;

    return IR_DISTANCE_OK;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_CalibratePoint(IR_DISTANCE_Handle_t *hird,
                                                     uint16_t distance,
                                                     uint16_t adcValue)
{
    IR_DISTANCE_CustomCurve_t candidate;
    IR_DISTANCE_StatusTypeDef status;

    IR_DISTANCE_CHECK_HANDLE(hird);

    /* Build the new curve aside: a point that breaks the monotonic ordering
       would make every later interpolation return garbage. */
    candidate = hird->customCurve;

    status = IR_DISTANCE_CURVE_Insert(&candidate, distance, adcValue);
    if (status != IR_DISTANCE_OK) {
        return status;
    }

    status = IR_DISTANCE_CURVE_Validate(&candidate);
    if (status != IR_DISTANCE_OK) {
        log_error("IR_DISTANCE: calibration point %u mm / ADC %u breaks the curve ordering",
                  distance, adcValue);
        return status;
    }

    /* sensorType is left alone: trimming a point does not turn a Sharp module
       into a different sensor, and it is what ResetCalibration keys off. */
    hird->customCurve = candidate;

    return IR_DISTANCE_OK;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_ResetCalibration(IR_DISTANCE_Handle_t *hird)
{
    const IR_DISTANCE_CustomCurve_t *curve;

    IR_DISTANCE_CHECK_HANDLE(hird);

    curve = IR_DISTANCE_CURVE_GetPredefined(hird->config.sensorType);
    if (curve == NULL) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    hird->customCurve = *curve;

    return IR_DISTANCE_OK;
}
