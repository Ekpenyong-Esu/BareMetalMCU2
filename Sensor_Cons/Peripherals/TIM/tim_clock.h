/**
 * @file tim_clock.h
 * @brief Timer clock enable and rate query
 *
 * Single-responsibility module answering the two questions every timer user
 * has before configuring anything: how do I turn this timer on, and how fast
 * is the clock feeding it?
 */

#ifndef TIM_CLOCK_H
#define TIM_CLOCK_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Enable the peripheral clock of a timer
 * @param  instance Timer peripheral
 * @retval true on success, false if the timer is not present on this device
 */
bool TIM_Clock_Enable(const TIM_TypeDef *instance);

/**
 * @brief  Get the clock actually driving a timer's counter
 * @note   Not the same as the APB bus clock: whenever the APB prescaler is
 *         greater than 1 the timers receive twice their bus clock.
 * @param  instance Timer peripheral
 * @retval Timer clock in Hz
 */
uint32_t TIM_Clock_GetHz(const TIM_TypeDef *instance);

/**
 * @brief  Check whether a timer has output compare channels
 * @details TIM6 and TIM7 are basic timers and cannot drive a pin.
 * @param  instance Timer peripheral
 * @retval true if the timer can drive PWM or output compare
 */
bool TIM_Clock_HasOutputChannels(const TIM_TypeDef *instance);

#ifdef __cplusplus
}
#endif

#endif /* TIM_CLOCK_H */
