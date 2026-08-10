/**
 * @file tim_encoder.h
 * @brief Timer quadrature encoder interface driver (rotary encoder reading)
 *
 * Single-responsibility module for the encoder interface mode.
 */

#ifndef TIM_ENCODER_H
#define TIM_ENCODER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize timer for quadrature encoder interface
 * @param  htim      Timer handle
 * @param  instance  Timer peripheral (must support encoder: TIM1-5, TIM8)
 * @param  period    Max count before rollover (use 0xFFFF for 16-bit)
 */
HAL_StatusTypeDef TIM_Encoder_Init(TIM_HandleTypeDef *htim,
                                   TIM_TypeDef *instance,
                                   uint32_t period);

/**
 * @brief  Start encoder counting
 */
HAL_StatusTypeDef TIM_Encoder_Start(TIM_HandleTypeDef *htim);

/**
 * @brief  Start encoder counting with interrupt
 */
HAL_StatusTypeDef TIM_Encoder_Start_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop encoder counting
 */
HAL_StatusTypeDef TIM_Encoder_Stop(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop encoder counting interrupt
 */
HAL_StatusTypeDef TIM_Encoder_Stop_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Get encoder count (position)
 */
int32_t TIM_Encoder_GetCount(TIM_HandleTypeDef *htim);

/**
 * @brief  Reset encoder count to zero
 */
void TIM_Encoder_Reset(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* TIM_ENCODER_H */
