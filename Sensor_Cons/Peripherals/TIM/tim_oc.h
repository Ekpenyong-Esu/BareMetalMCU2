/**
 * @file tim_oc.h
 * @brief Timer output compare driver (timing events)
 *
 * Single-responsibility module for output compare on a timer channel.
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
 */
HAL_StatusTypeDef TIM_OC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period);

/**
 * @brief  Configure output compare channel
 * @param  htim    Timer handle
 * @param  channel Timer channel
 * @param  pulse   Compare value (when counter matches, output toggles)
 */
HAL_StatusTypeDef TIM_OC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t pulse);

/**
 * @brief  Start output compare
 */
HAL_StatusTypeDef TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop output compare
 */
HAL_StatusTypeDef TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* TIM_OC_H */
