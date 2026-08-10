/**
 * @file tim_pwm.h
 * @brief Timer PWM output driver (LED dimming, motor control)
 *
 * Single-responsibility module for PWM output on a timer channel.
 */

#ifndef TIM_PWM_H
#define TIM_PWM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize timer for PWM output
 * @param  htim      Timer handle
 * @param  instance  Timer peripheral
 * @param  prescaler Clock divider
 * @param  period    PWM period (frequency = timer_clk / (prescaler+1) / (period+1))
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef TIM_PWM_Init(TIM_HandleTypeDef *htim,
                               TIM_TypeDef *instance,
                               uint32_t prescaler,
                               uint32_t period);

/**
 * @brief  Initialize timer for PWM output at a given frequency
 * @details Enables the timer clock and derives the prescaler from the clock
 *          actually feeding the timer, so callers state what they want rather
 *          than computing dividers.
 * @param  htim        Timer handle
 * @param  instance    Timer peripheral; must have output channels
 * @param  frequencyHz Desired PWM carrier frequency
 * @param  steps       Duty resolution, i.e. compare values per period
 * @retval HAL_OK on success, HAL_ERROR if the timer has no output channels or
 *         the requested frequency is unreachable at this resolution
 */
HAL_StatusTypeDef TIM_PWM_InitHz(TIM_HandleTypeDef *htim,
                                 TIM_TypeDef *instance,
                                 uint32_t frequencyHz,
                                 uint32_t steps);

/**
 * @brief  Configure PWM channel duty cycle
 * @param  htim    Timer handle
 * @param  channel TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, or TIM_CHANNEL_4
 * @param  pulse   Compare value (duty = pulse / period * 100%)
 */
HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim,
                                        uint32_t channel,
                                        uint32_t pulse);

/**
 * @brief  Start PWM output on channel
 */
HAL_StatusTypeDef TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Start PWM output on channel with interrupt
 */
HAL_StatusTypeDef TIM_PWM_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop PWM output on channel
 */
HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop PWM output interrupt on channel
 */
HAL_StatusTypeDef TIM_PWM_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Set PWM duty cycle while running
 * @param  htim    Timer handle
 * @param  channel Timer channel
 * @param  pulse   New compare value
 */
void TIM_PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse);

#ifdef __cplusplus
}
#endif

#endif /* TIM_PWM_H */
