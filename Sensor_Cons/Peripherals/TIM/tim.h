/**
 * @file    tim.h
 * @brief   Timer driver - main header for all timer features
 * @details Timers count time. They can make delays, make PWM signals
 *          to dim LEDs or move motors, and measure incoming pulses.
 *          This file pulls in all timer parts. You can include this
 *          one file to get everything, or include just one part
 *          like tim_pwm.h if you only need PWM.
 */

#ifndef TIM_H
#define TIM_H

/* Public interface aggregator: each module header is self-contained and
 * brings in the HAL types it needs. Include tim.h for all timer features,
 * or a single module header (e.g. tim_pwm.h) for minimal dependencies. */
#include "tim_base.h"
#include "tim_clock.h"
#include "tim_pwm.h"
#include "tim_ic.h"
#include "tim_oc.h"
#include "tim_encoder.h"

#endif /* TIM_H */

