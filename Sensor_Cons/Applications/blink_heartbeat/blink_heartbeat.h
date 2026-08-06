/**
 * @file    blink_heartbeat.h
 * @brief   Application 1 - Blink & Heartbeat.
 *
 * Composes four independent LED behaviours, each in its own module:
 *   - blink_steady           : external LED (PB4)  on/off at a fixed rate
 *   - heartbeat_onoff        : red LED     (PG14) lub-dub, fully lit or dark
 *   - heartbeat_fade_software: green LED   (PG13) lub-dub, CPU-generated PWM
 *   - heartbeat_fade_hardware: external LED (PA5) lub-dub, TIM2-generated PWM
 *
 * The three heartbeat modules share one beat timing table (heartbeat_rhythm.h)
 * and differ only in how they turn a beat into light, which makes the cost of
 * bit-banging a waveform visible next to letting a timer do it.
 *
 * The application is fully non-blocking. It never calls a delay; all timing is
 * derived from HAL_GetTick(). This keeps the CPU free and lets the demo
 * cooperate with other tasks in a super-loop.
 *
 * Layering:
 *   main.c                        -> chooses and runs the application
 *   blink_heartbeat               -> composition only (this module)
 *   blink_steady / heartbeat_*    -> one LED behaviour each
 *   led_blink / led_pattern       -> reusable timing engines (drivers)
 *   led / led_pwm / led_pwm_timer -> LED state and PWM output stages (drivers)
 *   board.h                       -> which pins the LEDs are wired to
 */

#ifndef BLINK_HEARTBEAT_H
#define BLINK_HEARTBEAT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Start every behaviour, then tick them forever.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return; calls Error_Handler() if a behaviour fails to start.
 */
void BlinkHeartbeat_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_H */
