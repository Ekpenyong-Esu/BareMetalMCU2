/**
 * @file tim.h
 * @brief STM32F429I-DISC1 Timer Driver - public interface aggregator
 *
 * This header is the single public entry point for all timer features.
 * Each feature is implemented as its own single-responsibility module:
 * - tim_base.h    Base timer (counting, delays)
 * - tim_pwm.h     PWM output (LED dimming, motor control)
 * - tim_ic.h      Input capture (frequency/pulse measurement)
 * - tim_oc.h      Output compare (timing events)
 * - tim_encoder.h Encoder interface (rotary encoder reading)
 *
 * Consumers may include tim.h (everything) or only the module header
 * they need.
 *
 * @note Enable timer clock before calling init functions:
 *       __HAL_RCC_TIMx_CLK_ENABLE()
 */

#ifndef TIM_H
#define TIM_H

/* Public interface aggregator: each module header is self-contained and
 * brings in the HAL types it needs. Include tim.h for all timer features,
 * or a single module header (e.g. tim_pwm.h) for minimal dependencies. */
#include "tim_base.h"
#include "tim_pwm.h"
#include "tim_ic.h"
#include "tim_oc.h"
#include "tim_encoder.h"

#endif /* TIM_H */

