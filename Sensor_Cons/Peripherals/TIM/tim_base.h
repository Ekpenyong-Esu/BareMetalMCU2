/**
 * @file tim_base.h
 * @brief Timer base mode driver (counting, delays)
 *
 * Single-responsibility module for the base timer functionality:
 * initialize, start/stop, and read/write the counter.
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
 * @param  htim      Timer handle
 * @param  instance  Timer peripheral (TIM1, TIM2, TIM3, etc.)
 * @param  prescaler Clock divider (timer_clk = APB_clk / (prescaler + 1))
 * @param  period    Auto-reload value (overflow at period + 1 counts)
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef TIM_Init(TIM_HandleTypeDef *htim,
                           TIM_TypeDef *instance,
                           uint32_t prescaler,
                           uint32_t period);

/**
 * @brief  Start timer counting
 */
HAL_StatusTypeDef TIM_Start(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop timer counting
 */
HAL_StatusTypeDef TIM_Stop(TIM_HandleTypeDef *htim);

/**
 * @brief  Start timer with update interrupt (calls HAL_TIM_PeriodElapsedCallback)
 */
HAL_StatusTypeDef TIM_Start_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop timer interrupt
 */
HAL_StatusTypeDef TIM_Stop_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Get current counter value
 */
uint32_t TIM_GetCounter(TIM_HandleTypeDef *htim);

/**
 * @brief  Set counter value
 */
void TIM_SetCounter(TIM_HandleTypeDef *htim, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* TIM_BASE_H */
