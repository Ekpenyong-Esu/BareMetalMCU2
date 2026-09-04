/**
 * @file tim_encoder.h
 * @brief Timer quadrature encoder interface driver (rotary encoder reading)
 *
 * Single-responsibility module for the encoder interface mode.
 *
 * Encoder Interface Mode:
 * - Uses two timer inputs (TI1 and TI2) connected to encoder A/B channels
 * - Timer counts up/down based on quadrature phase relationship
 * - Provides 1x, 2x, or 4x resolution (this driver uses 4x by default)
 * - Only available on advanced/general-purpose timers: TIM1-5, TIM8
 *
 * Wiring:
 *   Encoder A -> TI1 (e.g., PA8 for TIM1_CH1)
 *   Encoder B -> TI2 (e.g., PA9 for TIM1_CH2)
 *   Encoder GND -> GND, VCC -> 3.3V/5V
 *
 * The timer acts as a position counter. Reading CNT gives the current
 * position (signed for direction). Overflow/underflow wraps at period.
 *
 * Usage Pattern:
 *   1. TIM_Encoder_Init() with period (typically 0xFFFF for 16-bit)
 *   2. TIM_Encoder_Start() to begin counting
 *   3. TIM_Encoder_GetCount() to read position (returns int32_t)
 *   4. TIM_Encoder_Reset() to zero the position
 *   5. TIM_Encoder_Stop() to stop
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
 *
 * Configures the timer in encoder mode 3 (count on both TI1 and TI2 edges,
 * giving 4x resolution). The timer clock must be enabled beforehand.
 * Only TIM1-5 and TIM8 support encoder mode.
 *
 * @param  htim      Timer handle (uninitialized)
 * @param  instance  Timer peripheral (must support encoder: TIM1-5, TIM8)
 * @param  period    Max count before rollover (use 0xFFFF for 16-bit timers,
 *                   0xFFFFFFFF for 32-bit TIM2/TIM5)
 * @retval HAL_OK on success, HAL_ERROR if arguments invalid or HAL init fails
 */
HAL_StatusTypeDef TIM_Encoder_Init(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t period);

/**
 * @brief  Start encoder counting
 *
 * Enables the encoder interface and starts the counter.
 *
 * @param  htim  Timer handle (must be initialized)
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Encoder_Start(TIM_HandleTypeDef *htim);

/**
 * @brief  Start encoder counting with interrupt
 *
 * Same as TIM_Encoder_Start() but also enables the update interrupt.
 * HAL_TIM_PeriodElapsedCallback() is called on overflow/underflow.
 *
 * @param  htim  Timer handle
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Encoder_Start_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop encoder counting
 *
 * Disables the encoder interface and stops the counter.
 *
 * @param  htim  Timer handle
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Encoder_Stop(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop encoder counting interrupt
 *
 * Disables the update interrupt but leaves the encoder running.
 *
 * @param  htim  Timer handle
 * @retval HAL_OK on success, HAL_ERROR if htim is NULL
 */
HAL_StatusTypeDef TIM_Encoder_Stop_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Get encoder count (position)
 *
 * Reads the CNT register and returns it as a signed 32-bit value.
 * For 16-bit timers, the value is sign-extended from 16 bits.
 * Positive = forward rotation, negative = reverse.
 *
 * @param  htim  Timer handle
 * @retval Current encoder position (signed), or 0 if htim is NULL
 */
int32_t TIM_Encoder_GetCount(TIM_HandleTypeDef *htim);

/**
 * @brief  Reset encoder count to zero
 *
 * Sets the CNT register to 0. Useful for setting a reference position.
 *
 * @param  htim  Timer handle
 */
void TIM_Encoder_Reset(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* TIM_ENCODER_H */
