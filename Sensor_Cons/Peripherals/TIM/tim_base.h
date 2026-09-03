/**
 * @file tim_base.h
 * @brief Timer base mode driver (counting, delays, time-base generation)
 *
 * Single-responsibility module for the base timer functionality:
 * initialize, start/stop, and read/write the counter.
 *
 * Base Timer Use Cases:
 * - Simple delay loops (polling the counter)
 * - Time-base for periodic tasks (with update interrupt)
 * - Measuring elapsed time between events
 * - Free-running counter for timestamps
 *
 * The timer counts up from 0 to period, then overflows to 0 (generating UEV).
 * Timer clock = timer_input_clk / (prescaler + 1)
 * Overflow period = (period + 1) / timer_clock seconds
 *
 * Usage Pattern:
 *   1. TIM_Init() with desired prescaler and period
 *   2. TIM_Start() or TIM_Start_IT() to begin counting
 *   3. TIM_GetCounter() / TIM_SetCounter() to read/write
 *   4. TIM_Stop() to stop
 */

#ifndef TIM_BASE_H
#define TIM_BASE_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize timer as basic up-counter
 *
 * Configures the timer in up-counting mode with the given prescaler and
 * period. The timer clock must be enabled beforehand (TIM_Clock_Enable()).
 * Auto-reload preload is disabled so period changes take effect immediately.
 *
 * @param  htim      Timer handle (uninitialized)
 * @param  instance  Timer peripheral (TIM1..TIM14)
 * @param  prescaler Clock divider (timer_clk = timer_input_clk / (prescaler + 1))
 * @param  period    Auto-reload value (overflow at period + 1 counts)
 * @retval HAL_OK on success, HAL_ERROR if arguments invalid or HAL init fails
 */
HAL_StatusTypeDef TIM_Init(TIM_HandleTypeDef *htim,
                           TIM_TypeDef *instance,
                           uint32_t prescaler,
                           uint32_t period);

/**
 * @brief  Start timer counting (polling mode)
 *
 * Enables the counter. The timer begins counting from the current CNT value
 * (0 after init, or whatever TIM_SetCounter() last set).
 *
 * @param  htim  Timer handle (must be initialized)
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Start(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop timer counting
 *
 * Disables the counter. The current CNT value is preserved.
 *
 * @param  htim  Timer handle
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Stop(TIM_HandleTypeDef *htim);

/**
 * @brief  Start timer with update interrupt (UEV)
 *
 * Enables the counter and the update interrupt. On each overflow (CNT == ARR),
 * HAL_TIM_PeriodElapsedCallback() is called. Useful for periodic tasks.
 *
 * @param  htim  Timer handle (must be initialized)
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Start_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop timer interrupt
 *
 * Disables the update interrupt but leaves the counter running.
 *
 * @param  htim  Timer handle
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Stop_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Get current counter value
 *
 * Reads the CNT register. For 16-bit timers (TIM1-5, TIM8-11), returns
 * 0..65535. For 32-bit timers (TIM2, TIM5), returns 0..0xFFFFFFFF.
 *
 * @param  htim  Timer handle
 * @retval Current counter value, or 0 if htim is NULL
 */
uint32_t TIM_GetCounter(TIM_HandleTypeDef *htim);

/**
 * @brief  Set counter value
 *
 * Writes the CNT register. Useful for synchronizing timers or resetting
 * a free-running counter to a known value.
 *
 * @param  htim   Timer handle
 * @param  value  New counter value
 */
void TIM_SetCounter(TIM_HandleTypeDef *htim, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* TIM_BASE_H */
