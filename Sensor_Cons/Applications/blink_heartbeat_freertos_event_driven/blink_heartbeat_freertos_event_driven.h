/**
 * @file    blink_heartbeat_freertos_event_driven.h
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS, event-driven.
 *
 * Same four LED behaviours as blink_heartbeat, each in its own FreeRTOS task.
 * Tasks never poll: each blocks on a task notification, and a software timer
 * per behaviour fires that notification at the behaviour's cadence.
 */

#ifndef BLINK_HEARTBEAT_FREERTOS_EVENT_DRIVEN_H
#define BLINK_HEARTBEAT_FREERTOS_EVENT_DRIVEN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise every behaviour, create one task + one timer per
 *         behaviour, and start the FreeRTOS scheduler.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return on success; calls Error_Handler() on failure.
 */
void BlinkHeartbeatFreeRTOSEventDriven_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_FREERTOS_EVENT_DRIVEN_H */
