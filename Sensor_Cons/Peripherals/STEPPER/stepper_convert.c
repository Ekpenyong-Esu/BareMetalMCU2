/**
 ******************************************************************************
 * @file    stepper_convert.c
 * @brief   Speed and step-interval conversions
 ******************************************************************************
 */

#include "stepper_convert.h"

#define STEPPER_SECONDS_PER_MINUTE 60UL
#define STEPPER_US_PER_SECOND 1000000UL

uint32_t STEPPER_RPMToDelay(uint16_t rpm, uint16_t stepsPerRev) {
    uint32_t delay = 0;

    if (rpm == 0U || stepsPerRev == 0U) {
        return STEPPER_MAX_DELAY_US;
    }

    delay = (STEPPER_SECONDS_PER_MINUTE * STEPPER_US_PER_SECOND) /
            ((uint32_t)rpm * (uint32_t)stepsPerRev);

    if (delay < STEPPER_MIN_DELAY_US) {
        delay = STEPPER_MIN_DELAY_US;
    }
    else if (delay > STEPPER_MAX_DELAY_US) {
        delay = STEPPER_MAX_DELAY_US;
    }

    return delay;
}

uint16_t STEPPER_DelayToRPM(uint32_t delay, uint16_t stepsPerRev) {
    uint32_t rpm = 0;

    if (delay == 0U || stepsPerRev == 0U) {
        return STEPPER_MIN_SPEED_RPM;
    }

    rpm = (STEPPER_SECONDS_PER_MINUTE * STEPPER_US_PER_SECOND) / (delay * (uint32_t)stepsPerRev);

    if (rpm < STEPPER_MIN_SPEED_RPM) {
        rpm = STEPPER_MIN_SPEED_RPM;
    }
    else if (rpm > STEPPER_MAX_SPEED_RPM) {
        rpm = STEPPER_MAX_SPEED_RPM;
    }

    return (uint16_t)rpm;
}
