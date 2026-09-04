/**
 ******************************************************************************
 * @file    ultrasonic_measure.c
 * @brief   Ultrasonic measurement cycle
 ******************************************************************************
 */

#include "ultrasonic_measure.h"
#include "ultrasonic_capture.h"
#include "ultrasonic_convert.h"
#include "ultrasonic_gpio.h"
#include "tim_ic.h"
#include "log.h"

ULTRASONIC_StatusTypeDef ULTRASONIC_StartMeasurement(ULTRASONIC_Handle_t *hultra) {
    ULTRASONIC_StatusTypeDef status = ULTRASONIC_OK;

    ULTRASONIC_CHECK_HANDLE(hultra);

    if (hultra->echoState != ULTRASONIC_ECHO_IDLE) {
        log_warning("ULTRASONIC: Sensor busy, previous measurement not complete");
        return ULTRASONIC_BUSY;
    }

    hultra->measurementDone = false;
    hultra->echoStart = 0;
    hultra->echoTicks = 0;
    hultra->echoState = ULTRASONIC_ECHO_WAIT_RISING;

    status = ULTRASONIC_CAPTURE_Arm(hultra->htim, hultra->channel);
    if (status != ULTRASONIC_OK) {
        hultra->echoState = ULTRASONIC_ECHO_IDLE;
        hultra->measurementDone = true;
        return status;
    }

    ULTRASONIC_GPIO_SetTrigger(&hultra->pins, true);
    ULTRASONIC_CAPTURE_DelayMicroseconds(hultra->htim, ULTRASONIC_TRIGGER_PULSE_WIDTH_US);
    ULTRASONIC_GPIO_SetTrigger(&hultra->pins, false);

    return ULTRASONIC_OK;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_AbortMeasurement(ULTRASONIC_Handle_t *hultra) {
    ULTRASONIC_CHECK_HANDLE(hultra);

    (void)ULTRASONIC_CAPTURE_Disarm(hultra->htim, hultra->channel);

    hultra->echoState = ULTRASONIC_ECHO_IDLE;
    hultra->echoStart = 0;
    hultra->echoTicks = 0;
    hultra->measurementDone = true;

    return ULTRASONIC_OK;
}

bool ULTRASONIC_IsMeasurementComplete(const ULTRASONIC_Handle_t *hultra) {
    return (hultra != NULL) ? hultra->measurementDone : false;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_WaitForMeasurement(ULTRASONIC_Handle_t *hultra,
                                                       uint32_t timeout) {
    uint32_t startTime = 0;

    ULTRASONIC_CHECK_HANDLE(hultra);

    startTime = HAL_GetTick();

    while (!hultra->measurementDone) {
        if ((HAL_GetTick() - startTime) > timeout) {
            /* Leaving the channel armed on a falling edge would make the next
               cycle interpret its first capture as an echo end. */
            (void)ULTRASONIC_AbortMeasurement(hultra);
            return ULTRASONIC_TIMEOUT;
        }
    }

    return ULTRASONIC_OK;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_Measure(ULTRASONIC_Handle_t *hultra, uint16_t *distance) {
    ULTRASONIC_StatusTypeDef status = ULTRASONIC_OK;
    uint16_t measured = 0;

    ULTRASONIC_CHECK_HANDLE(hultra);

    hultra->lastDistance = 0;

    status = ULTRASONIC_StartMeasurement(hultra);
    if (status != ULTRASONIC_OK) {
        return status;
    }

    status = ULTRASONIC_WaitForMeasurement(hultra, hultra->config.measurementTimeout);
    if (status != ULTRASONIC_OK) {
        return status;
    }

    measured = ULTRASONIC_EchoToDistance(hultra->echoTicks, hultra->config.temperature);

    if (!ULTRASONIC_IsValidDistance(hultra, measured)) {
        log_warning("ULTRASONIC: echo of %lu us gives %u mm, outside the configured range",
                    (unsigned long)hultra->echoTicks, measured);
        return ULTRASONIC_OUT_OF_RANGE;
    }

    hultra->lastDistance = measured;
    if (distance != NULL) {
        *distance = measured;
    }

    return ULTRASONIC_OK;
}

uint16_t ULTRASONIC_MeasureDistance(ULTRASONIC_Handle_t *hultra) {
    uint16_t distance = 0;

    (void)ULTRASONIC_Measure(hultra, &distance);

    return distance;
}

uint16_t ULTRASONIC_GetDistance(const ULTRASONIC_Handle_t *hultra) {
    return (hultra != NULL) ? hultra->lastDistance : 0U;
}

void ULTRASONIC_TIM_IC_CaptureCallback(ULTRASONIC_Handle_t *hultra) {
    uint32_t capture = 0;

    if (hultra == NULL || !hultra->isInitialized) {
        return;
    }

    capture = TIM_IC_GetCapture(hultra->htim, hultra->channel);

    switch (hultra->echoState) {
        case ULTRASONIC_ECHO_WAIT_RISING:
            hultra->echoStart = capture;
            hultra->echoState = ULTRASONIC_ECHO_WAIT_FALLING;
            (void)ULTRASONIC_CAPTURE_ExpectFalling(hultra->htim, hultra->channel);
            break;

        case ULTRASONIC_ECHO_WAIT_FALLING:
            /* Only the raw width is taken here; the conversion and the logging
               belong to the caller, not to interrupt context. */
            hultra->echoTicks =
                ULTRASONIC_EchoWidth(hultra->echoStart, capture, ULTRASONIC_ECHO_PERIOD);
            hultra->echoState = ULTRASONIC_ECHO_IDLE;
            hultra->measurementDone = true;
            (void)ULTRASONIC_CAPTURE_Disarm(hultra->htim, hultra->channel);
            break;

        default:
            /* Spurious capture outside a measurement cycle. */
            (void)ULTRASONIC_CAPTURE_Disarm(hultra->htim, hultra->channel);
            break;
    }
}
