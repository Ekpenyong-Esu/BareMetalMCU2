/**
 * @file    blink_heartbeat.h
 * @brief   Application 1 - Blink & Heartbeat.
 *
 * A self-contained demo for the STM32F429I-DISC1 on-board LEDs:
 *   - Green LED (PG13): alternates every few seconds between a fixed-rate
 *     "activity" blink and a breathing fade driven by software PWM.
 *   - Red LED   (PG14): "heartbeat" double-pulse pattern (lub-dub ... rest).
 *
 * The on-board LEDs have no timer alternate function, so the breathing effect
 * is produced by the cooperative software-PWM module rather than a TIM channel.
 *
 * The application is fully non-blocking. It never calls a delay; all timing is
 * derived from HAL_GetTick() and the microsecond counter in SYS. This keeps the
 * CPU free and lets the demo cooperate with other tasks in a super-loop.
 *
 * Layering:
 *   main.c                    -> chooses and runs the application
 *   blink_heartbeat           -> orchestration + phase alternation (this module)
 *   heartbeat                 -> lub-dub pattern player (reusable)
 *   led_blink / led_pwm       -> time-driven LED behaviours
 *   led (driver)              -> LED on/off state
 *   board.h                   -> which pins the LEDs are wired to
 */

#ifndef APP_BLINK_HEARTBEAT_H
#define APP_BLINK_HEARTBEAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief  Initialise the on-board LEDs and start both patterns.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 * @retval true on success, false if an LED could not be initialised.
 */
bool App_BlinkHeartbeat_Init(void);

/**
 * @brief  Advance the LED patterns. Call once per super-loop iteration.
 * @note   Non-blocking; returns immediately when no LED needs updating.
 */
void App_BlinkHeartbeat_Task(void);

/**
 * @brief  Convenience entry point: initialise, then run the task loop forever.
 * @note   Does not return.
 */
void App_BlinkHeartbeat_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLINK_HEARTBEAT_H */
