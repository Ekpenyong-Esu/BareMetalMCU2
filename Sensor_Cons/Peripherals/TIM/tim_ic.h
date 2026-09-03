/**
 * @file tim_ic.h
 * @brief Timer input capture driver (frequency/pulse measurement, pulse width)
 *
 * Single-responsibility module for input capture on a timer channel.
 *
 * Input Capture Use Cases:
 * - Frequency measurement (period between rising edges)
 * - Pulse width measurement (time between rising and falling edges)
 * - Duty cycle measurement (capture on both edges)
 * - External event timestamping
 *
 * The timer runs as a free-running counter. On the selected edge (rising,
 * falling, or both), the current CNT value is latched into CCR. By reading
 * consecutive captures, the time between edges can be computed.
 *
 * Usage Pattern:
 *   1. TIM_IC_Init() with prescaler/period for desired time resolution
 *   2. TIM_IC_ConfigChannel() with desired edge polarity
 *   3. TIM_IC_Start() or TIM_IC_Start_IT() to begin capturing
 *   4. TIM_IC_GetCapture() to read captured values
 *   5. TIM_IC_Stop() to stop
 *
 * Frequency Measurement:
 *   freq = timer_clk / (capture2 - capture1)  (handle overflow)
 * Pulse Width:
 *   width = (capture_falling - capture_rising) / timer_clk
 */

#ifndef TIM_IC_H
#define TIM_IC_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize timer for input capture
 *
 * Configures the timer as a free-running up-counter. The timer clock must
 * be enabled beforehand (TIM_Clock_Enable()). The prescaler determines the
 * time resolution: timer_clk = timer_input_clk / (prescaler + 1).
 *
 * @param  htim      Timer handle (uninitialized)
 * @param  instance  Timer peripheral (must have input capture channels)
 * @param  prescaler Clock divider for time resolution
 * @param  period    Auto-reload value (max count before overflow)
 * @retval HAL_OK on success, HAL_ERROR if arguments invalid or HAL init fails
 */
HAL_StatusTypeDef TIM_IC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period);

/**
 * @brief  Configure input capture channel
 *
 * Sets up the channel for input capture on the specified edge polarity.
 * The input filter and prescaler are left at defaults (no filter, no divider).
 *
 * @param  htim     Timer handle (must be initialized)
 * @param  channel  Timer channel (TIM_CHANNEL_1..4)
 * @param  polarity TIM_ICPOLARITY_RISING, TIM_ICPOLARITY_FALLING,
 *                  or TIM_ICPOLARITY_BOTHEDGE
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL or HAL config fails
 */
HAL_StatusTypeDef TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t polarity);

/**
 * @brief  Start input capture (polling mode)
 *
 * Enables the capture channel. The CCR register will be updated on each
 * selected edge. The caller must poll or use interrupts to read captures
 * before the next edge overwrites them.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Start input capture with interrupt
 *
 * Same as TIM_IC_Start() but also enables the capture interrupt (CCxIE).
 * HAL_TIM_IC_CaptureCallback() is called on each capture event.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop input capture
 *
 * Disables the capture channel.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_IC_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop input capture interrupt
 *
 * Disables the capture interrupt but leaves the channel enabled.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_IC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Get captured value
 *
 * Reads the CCR register for the channel. This is the timer counter value
 * latched at the last capture event.
 *
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @retval Captured counter value, or 0 if htim is NULL
 */
uint32_t TIM_IC_GetCapture(TIM_HandleTypeDef *htim, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* TIM_IC_H */
