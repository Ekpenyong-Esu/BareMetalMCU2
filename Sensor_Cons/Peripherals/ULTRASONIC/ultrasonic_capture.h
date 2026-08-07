/**
  ******************************************************************************
  * @file    ultrasonic_capture.h
  * @brief   Capture timer setup and microsecond timing (internal)
  * @details Not part of the public ultrasonic.h aggregator.
  ******************************************************************************
  */

#ifndef ULTRASONIC_CAPTURE_H
#define ULTRASONIC_CAPTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ultrasonic_types.h"

/**
 * @brief   Program the timer as a free running 1 MHz counter and set the
 *          capture channel to detect the rising edge of the echo.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_Init(TIM_HandleTypeDef *htim, uint32_t channel);

ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_DeInit(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief   Arm the capture interrupt for a rising edge.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_Arm(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief   Disarm the capture interrupt and restore the rising edge polarity.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_Disarm(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief   Switch the capture channel to the falling edge of the echo.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_ExpectFalling(TIM_HandleTypeDef *htim,
                                                          uint32_t channel);

/**
 * @brief   Busy wait against the capture counter, which already ticks at 1 MHz.
 */
void ULTRASONIC_CAPTURE_DelayMicroseconds(TIM_HandleTypeDef *htim, uint32_t microseconds);

#ifdef __cplusplus
}
#endif

#endif /* ULTRASONIC_CAPTURE_H */
