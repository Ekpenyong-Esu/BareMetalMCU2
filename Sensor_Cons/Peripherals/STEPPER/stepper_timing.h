/**
 ******************************************************************************
 * @file    stepper_timing.h
 * @brief   Microsecond time base backed by the caller's timer
 * @note    Internal to the driver; not part of stepper.h.
 ******************************************************************************
 */

#ifndef STEPPER_TIMING_H
#define STEPPER_TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stepper_types.h"

/**
 * @brief Reprogram @p htim as a free-running 1 MHz counter and start it.
 * @note  The caller must have set htim->Instance; everything else is overwritten.
 */
STEPPER_StatusTypeDef STEPPER_TIMING_Init(TIM_HandleTypeDef *htim);

/**
 * @brief Stop the time base.
 */
void STEPPER_TIMING_DeInit(TIM_HandleTypeDef *htim);

/**
 * @brief Block for @p microseconds against the free-running counter.
 */
void STEPPER_TIMING_DelayMicroseconds(TIM_HandleTypeDef *htim, uint32_t microseconds);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_TIMING_H */
