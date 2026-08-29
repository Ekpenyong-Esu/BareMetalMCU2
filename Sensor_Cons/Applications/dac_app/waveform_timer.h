/**
 * @file waveform_timer.h
 * @brief TIM7 pacing for the waveform app (TRGO routing + update interrupt)
 *
 * TIM7 rather than TIM6: the HAL timebase already owns TIM6 (see
 * Core/Src/stm32f4xx_hal_timebase_tim.c) and its handle is serviced by
 * TIM6_DAC_IRQHandler, so a second owner would fight it for HAL_GetTick.
 *
 * The handle is module-private because the update ISR needs it and no caller
 * does. Ticks are delivered through a callback so this module stays free of
 * any RTOS dependency.
 */

#ifndef WAVEFORM_TIMER_H
#define WAVEFORM_TIMER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Called from ISR context once per update event. */
typedef void (*Waveform_TimerTickFn)(void);

/** Bring TIM7 up as a 2 ms up-counter and route its update event to TRGO. */
bool Waveform_TimerInit(void);

/** Register the ISR-context tick handler. Call before Waveform_TimerStart(). */
void Waveform_TimerOnTick(Waveform_TimerTickFn onTick);

/** Enable the update interrupt and start counting. */
bool Waveform_TimerStart(void);

/** Service the TIM7 update event. Call only from TIM7_IRQHandler. */
void Waveform_TimerIrqHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_TIMER_H */
