/**
 * @file tim_oc.h
 * @brief Timer output compare driver (timing events, one-pulse mode)
 *
 * Single-responsibility module for output compare on a timer channel.
 *
 * Output Compare Use Cases:
 * - Generate a single pulse of precise width (one-pulse mode)
 * - Trigger an event at a specific time (toggle/set/clear output)
 * - Generate periodic interrupts at exact intervals
 * - Drive external hardware with precise timing
 *
 * In output compare mode, the timer counts up and when CNT matches CCR,
 * the output can toggle, set high, set low, or do nothing (interrupt only).
 * This is different from PWM where the output toggles at both CNT=0 and CNT=CCR.
 *
 * Usage Pattern:
 *   1. TIM_OC_Init() with prescaler/period
 *   2. TIM_OC_ConfigChannel() with compare value (pulse) and mode
 *   3. TIM_OC_Start() or TIM_OC_Start_IT() to begin
 *   4. TIM_OC_Stop() to stop
 *
 * One-Pulse Mode (OPM):
 *   Set TIM_OPMODE_SINGLE in the timer init to stop after one pulse.
 *   Useful for generating a single trigger pulse of exact width.
 */

#ifndef TIM_OC_H
#define TIM_OC_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize timer for output compare
 *
 * Configures the timer as an up-counter. The timer clock must be enabled
 * beforehand (TIM_Clock_Enable()). Auto-reload preload is disabled.
 *
 * @param  htim      Timer handle (uninitialized)
 * @param  instance  Timer peripheral (must have output compare channels)
 * @param  prescaler Clock divider
 * @param  period    Auto-reload value (max count)
 * @retval HAL_OK on success, HAL_ERROR if arguments invalid or HAL init fails
 */
HAL_StatusTypeDef TIM_OC_Init(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t prescaler,
                              uint32_t period);

/**
 * @brief  Configure output compare channel
 *
 * Sets up the channel in output compare mode. The default mode is
 * TIM_OCMODE_TOGGLE (output toggles on match). Other modes include
 * TIM_OCMODE_ACTIVE (set high), TIM_OCMODE_INACTIVE (set low),
 * TIM_OCMODE_FORCED_ACTIVE, TIM_OCMODE_FORCED_INACTIVE.
 *
 * @param  htim    Timer handle (must be initialized)
 * @param  channel Timer channel (TIM_CHANNEL_1..4)
 * @param  pulse   Compare value (CCR). When CNT == CCR, the output action occurs.
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL or HAL config fails
 */
HAL_StatusTypeDef TIM_OC_ConfigChannel(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse);

/**
 * @brief  Start output compare (polling mode)
 *
 * Enables the output compare channel and starts the counter.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Start output compare with interrupt
 *
 * Same as TIM_OC_Start() but also enables the compare interrupt (CCxIE).
 * HAL_TIM_OC_DelayElapsedCallback() is called on each match.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_OC_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop output compare
 *
 * Disables the output compare channel and stops the counter.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop output compare interrupt
 *
 * Disables the compare interrupt but leaves the channel enabled.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_OC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* TIM_OC_H */
