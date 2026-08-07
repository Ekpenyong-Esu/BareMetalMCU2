/**
  ******************************************************************************
  * @file    stepper_convert.h
  * @brief   Speed and step-interval conversions
  ******************************************************************************
  */

#ifndef STEPPER_CONVERT_H
#define STEPPER_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stepper_types.h"

/**
 * @brief  Step interval in microseconds for a given speed.
 * @note   Clamped to [STEPPER_MIN_DELAY_US, STEPPER_MAX_DELAY_US].
 */
uint32_t STEPPER_RPMToDelay(uint16_t rpm, uint16_t stepsPerRev);

/**
 * @brief  Speed in RPM for a given step interval.
 * @note   Clamped to [STEPPER_MIN_SPEED_RPM, STEPPER_MAX_SPEED_RPM].
 */
uint16_t STEPPER_DelayToRPM(uint32_t delay, uint16_t stepsPerRev);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_CONVERT_H */
