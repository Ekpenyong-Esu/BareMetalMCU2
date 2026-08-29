/**
 * @file waveform_timer.h
 * @brief TIM6 pacing for the waveform app (up-count + TRGO routing)
 */

#ifndef WAVEFORM_TIMER_H
#define WAVEFORM_TIMER_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bring TIM6 up as a plain 2 ms up-counter. */
bool Waveform_TimerInit(TIM_HandleTypeDef *tim);

/** Route TIM6's update event to its TRGO output. */
bool Waveform_TimerRouteTrigger(TIM_HandleTypeDef *tim);

/** Poll TIM6 update flag. */
bool Waveform_TimerTickArrived(TIM_HandleTypeDef *tim);

/** Clear TIM6 update flag. */
void Waveform_TimerAckTick(TIM_HandleTypeDef *tim);

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_TIMER_H */
