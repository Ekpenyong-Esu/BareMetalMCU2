/**
 * @file tim_pwm.h
 * @brief Timer PWM output driver (LED dimming, motor control, buzzer tones)
 *
 * Single-responsibility module for PWM output on a timer channel.
 *
 * PWM Frequency Formula:
 *   f_PWM = timer_clk / (prescaler + 1) / (period + 1)
 *   duty_cycle = pulse / (period + 1) * 100%
 *
 * The timer_clk is the timer input clock (after APB prescaler doubling),
 * obtained via TIM_Clock_GetHz(). TIM_PWM_InitHz() computes the prescaler
 * automatically for a target frequency and resolution (steps).
 *
 * Usage Pattern:
 *   1. TIM_PWM_InitHz() or TIM_PWM_Init() to configure the time base
 *   2. TIM_PWM_ConfigChannel() to set up each channel's duty/polarity
 *   3. TIM_PWM_Start() to begin output
 *   4. TIM_PWM_SetDuty() to change duty cycle on the fly
 *   5. TIM_PWM_Stop() to stop
 */

#ifndef TIM_PWM_H
#define TIM_PWM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize timer for PWM output with explicit prescaler and period
 *
 * Configures the timer in PWM mode 1 (up-counting, active high). The timer
 * clock must be enabled beforehand (TIM_Clock_Enable()).
 *
 * @param  htim      Timer handle (uninitialized)
 * @param  instance  Timer peripheral (must have output channels)
 * @param  prescaler Clock divider (timer_clk = timer_input_clk / (prescaler + 1))
 * @param  period    Auto-reload value (PWM period = period + 1 timer clocks)
 * @retval HAL_OK on success, HAL_ERROR if arguments invalid or HAL init fails
 */
HAL_StatusTypeDef TIM_PWM_Init(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t prescaler,
                               uint32_t period);

/**
 * @brief  Initialize timer for PWM output at a given frequency and resolution
 *
 * Convenience function that enables the timer clock, computes the prescaler
 * from the actual timer input clock, and initializes the timer. Callers
 * specify the desired PWM frequency and duty resolution (steps per period)
 * rather than raw register values.
 *
 * @param  htim        Timer handle (uninitialized)
 * @param  instance    Timer peripheral; must have output channels
 * @param  frequencyHz Desired PWM carrier frequency in Hz
 * @param  steps       Duty resolution (number of compare values per period).
 *                     Higher = finer duty control but lower max frequency.
 * @retval HAL_OK on success, HAL_ERROR if:
 *         - timer has no output channels (TIM6/TIM7)
 *         - frequency * steps exceeds timer clock (divider would be 0)
 *         - divider exceeds 16-bit prescaler limit (65536)
 *         - HAL initialization fails
 */
HAL_StatusTypeDef TIM_PWM_InitHz(TIM_HandleTypeDef *htim, TIM_TypeDef *instance,
                                 uint32_t frequencyHz, uint32_t steps);

/**
 * @brief  Configure a PWM channel's duty cycle and polarity
 *
 * Sets up the output compare channel in PWM mode 1. The pulse parameter
 * is the compare value (CCR); duty cycle = pulse / (period + 1).
 * Polarity can be inverted for active-low drivers (e.g., some MOSFET gates).
 *
 * @param  htim     Timer handle (must be initialized)
 * @param  channel  TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, or TIM_CHANNEL_4
 * @param  pulse    Compare value (0..period). 0 = always low, period = always high.
 * @param  polarity TIM_OCPOLARITY_HIGH (active high) or TIM_OCPOLARITY_LOW (active low)
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL or HAL config fails
 */
HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse,
                                        uint32_t polarity);

/**
 * @brief  Start PWM output on a channel (polling mode)
 *
 * Enables the output compare preload and starts the timer counter.
 * The output begins toggling according to the configured duty cycle.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Start PWM output on a channel with update interrupt
 *
 * Same as TIM_PWM_Start() but also enables the update interrupt (UEV).
 * The HAL_TIM_PeriodElapsedCallback will be called on each period.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_PWM_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop PWM output on a channel
 *
 * Disables the output compare channel and stops the timer counter.
 * The output pin returns to its reset state (low for active-high polarity).
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop PWM output interrupt on a channel
 *
 * Disables the update interrupt but leaves the PWM running.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_PWM_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Change PWM duty cycle on the fly
 *
 * Updates the compare register (CCR) immediately. The new duty takes effect
 * at the next counter overflow (or immediately if preload is disabled).
 * This is a lightweight operation suitable for real-time control loops.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @param  pulse    New compare value (0..period)
 */
void TIM_PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse);

#ifdef __cplusplus
}
#endif

#endif /* TIM_PWM_H */
