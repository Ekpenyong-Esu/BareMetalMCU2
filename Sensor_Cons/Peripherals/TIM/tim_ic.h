/**
 * @file tim_ic.h
 * @brief Timer input capture driver (frequency/pulse measurement)
 *
 * Single-responsibility module for input capture on a timer channel.
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
 */
HAL_StatusTypeDef TIM_IC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period);

/**
 * @brief  Configure input capture channel
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @param  polarity TIM_ICPOLARITY_RISING, TIM_ICPOLARITY_FALLING, or TIM_ICPOLARITY_BOTHEDGE
 */
HAL_StatusTypeDef TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t polarity);

/**
 * @brief  Start input capture
 */
HAL_StatusTypeDef TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Start input capture with interrupt
 */
HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop input capture
 */
HAL_StatusTypeDef TIM_IC_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop input capture interrupt
 */
HAL_StatusTypeDef TIM_IC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Get captured value
 */
uint32_t TIM_IC_GetCapture(TIM_HandleTypeDef *htim, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* TIM_IC_H */
