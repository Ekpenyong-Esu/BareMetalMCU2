/**
 * @file tim_clock.h
 * @brief Timer clock enable and rate query
 *
 * Single-responsibility module answering the two questions every timer user
 * has before configuring anything: how do I turn this timer on, and how fast
 * is the clock feeding it?
 *
 * STM32F4 Timer Clock Architecture:
 * - TIM1, TIM8, TIM9, TIM10, TIM11 are on APB2
 * - TIM2-7, TIM12-14 are on APB1
 * - When APB prescaler > 1, timers receive 2x the bus clock (PCLKx * 2)
 * - This module abstracts the RCC register bit-twiddling so callers just
 *   ask for the timer they want and get the correct clock rate.
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
 *
 * Sets the appropriate bit in RCC_APB1ENR or RCC_APB2ENR depending on
 * which bus the timer lives on. Call this before any other timer operation.
 *
 * @param  instance Timer peripheral (TIM1..TIM14)
 * @retval true on success, false if the timer is not present on this device
 */
bool TIM_Clock_Enable(const TIM_TypeDef *instance);

/**
 * @brief  Get the clock actually driving a timer's counter
 *
 * This is NOT the same as the APB bus clock. On STM32F4, whenever the APB
 * prescaler (PPRE1 or PPRE2) is greater than 1, the timers receive twice
 * their bus clock (PCLKx * 2). This function reads RCC_CFGR to determine
 * the current prescaler and returns the correct timer clock frequency.
 *
 * @note   Call TIM_Clock_Enable() first, or the clock may not be running.
 * @param  instance Timer peripheral (TIM1..TIM14)
 * @retval Timer clock in Hz (e.g., 84000000 for TIM9 on F429 at 168 MHz)
 */
uint32_t TIM_Clock_GetHz(const TIM_TypeDef *instance);

/**
 * @brief  Check whether a timer has output compare channels
 *
 * TIM6 and TIM7 are basic timers (16-bit, no output channels) used only for
 * time-base generation. All other timers (TIM1-5, TIM8-14) have at least
 * one output compare channel and can drive PWM or output compare signals.
 *
 * @param  instance Timer peripheral
 * @retval true if the timer can drive PWM or output compare, false for TIM6/TIM7
 */
bool TIM_Clock_HasOutputChannels(const TIM_TypeDef *instance);

#ifdef __cplusplus
}
#endif

#endif /* TIM_CLOCK_H */
