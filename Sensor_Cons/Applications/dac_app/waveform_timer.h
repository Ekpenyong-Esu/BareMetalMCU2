/**
 * @file waveform_timer.h
 * @brief The DAC's trigger source: TIM7 raising TRGO every 2 ms
 *
 * TIM7 rather than TIM6, because the HAL timebase already owns TIM6
 * (see Core/Src/stm32f4xx_hal_timebase_tim.c) and reprogramming it would
 * skew HAL_GetTick.
 *
 * Ticks leave through a plain function pointer so this module stays free of
 * any RTOS dependency.
 */

#ifndef WAVEFORM_TIMER_H
#define WAVEFORM_TIMER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Interval between TRGO pulses, i.e. between DAC output updates. */
#define WAVEFORM_SAMPLE_PERIOD_MS  2U

/** Called from ISR context once per update event. */
typedef void (*Waveform_TimerTickFn)(void);

/** Start TIM7, route its update event to TRGO, and report each tick to onTick. */
bool Waveform_TimerStart(Waveform_TimerTickFn onTick);

/** Service the TIM7 update event. Call only from TIM7_IRQHandler(). */
void Waveform_TimerIrqHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_TIMER_H */
