/**
 * @file    blink_heartbeat_freertos_event_groups.h
 * @brief   Application 4 - Blink & Heartbeat on FreeRTOS, event groups.
 *          Software timers set event flags; behaviour tasks block on them.
 */

#ifndef BLINK_HEARTBEAT_FREERTOS_EVENT_GROUPS_H
#define BLINK_HEARTBEAT_FREERTOS_EVENT_GROUPS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise every behaviour, create the timers, tasks and event
 *         group, and start the FreeRTOS scheduler.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return on success; calls Error_Handler() on failure.
 */
void BlinkHeartbeatFreeRTOSEventGroups_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_FREERTOS_EVENT_GROUPS_H */