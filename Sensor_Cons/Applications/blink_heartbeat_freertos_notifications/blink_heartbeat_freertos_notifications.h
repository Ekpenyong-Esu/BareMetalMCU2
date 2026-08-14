/**
 * @file    blink_heartbeat_freertos_notifications.h
 * @brief   Application 3 - Blink & Heartbeat on FreeRTOS, task notifications.
 *          Each behaviour task notifies a central monitor with its own bit.
 */

#ifndef BLINK_HEARTBEAT_FREERTOS_NOTIFICATIONS_H
#define BLINK_HEARTBEAT_FREERTOS_NOTIFICATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise every behaviour, create the tasks, and start the
 *         FreeRTOS scheduler.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return on success; calls Error_Handler() on failure.
 */
void BlinkHeartbeatFreeRTOSNotifications_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_FREERTOS_NOTIFICATIONS_H */