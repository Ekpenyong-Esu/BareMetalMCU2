/**
 * @file    blink_heartbeat_freertos_isr_offloading.h
 * @brief   Application 5 - Blink & Heartbeat on FreeRTOS, ISR offloading.
 *          A hardware timer ISR wakes behaviour tasks; they do the LED work.
 */

#ifndef BLINK_HEARTBEAT_FREERTOS_ISR_OFFLOADING_H
#define BLINK_HEARTBEAT_FREERTOS_ISR_OFFLOADING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise every behaviour, create the tasks, start the 1 kHz
 *         hardware timer that drives them, and start the FreeRTOS scheduler.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return on success; calls Error_Handler() on failure.
 */
void BlinkHeartbeatFreeRTOSIsrOffloading_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_FREERTOS_ISR_OFFLOADING_H */