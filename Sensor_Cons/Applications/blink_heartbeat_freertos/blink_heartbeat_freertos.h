/**
 * @file    blink_heartbeat_freertos.h
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS (raw API, no CMSIS).
 *
 * Same four LED behaviours as blink_heartbeat, but each behaviour runs in its
 * own FreeRTOS task instead of one shared super-loop:
 *   - blink_steady      : external LED (PB4)  on/off at a fixed rate
 *   - led_heartbeat     : red LED     (PG14) lub-dub, fully lit or dark
 *   - pwm_led_software  : green LED   (PG13) continuous breathe, CPU-generated PWM
 *   - pwm_led_hardware  : external LED (PA5) continuous breathe, TIM2-generated PWM
 *
 * The behaviour modules are unchanged and stay non-blocking; only the caller
 * changes. FreeRTOS owns SysTick for its 1 kHz tick, HAL keeps its 1 ms time
 * base on TIM6, so HAL_GetTick() keeps working for the behaviours.
 */

#ifndef BLINK_HEARTBEAT_FREERTOS_H
#define BLINK_HEARTBEAT_FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise every behaviour, create one task per behaviour and start
 *         the FreeRTOS scheduler.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return on success; calls Error_Handler() if a behaviour
 *         fails to start or the scheduler stops.
 */
void BlinkHeartbeatFreeRTOS_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_FREERTOS_H */
