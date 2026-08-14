/**
 * @file    blink_heartbeat_freertos_queues.h
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS, producer/consumer
 *          via a queue.
 */

#ifndef BLINK_HEARTBEAT_FREERTOS_QUEUES_H
#define BLINK_HEARTBEAT_FREERTOS_QUEUES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise every behaviour, create the producers + consumer, and
 *         start the FreeRTOS scheduler.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return on success; calls Error_Handler() on failure.
 */
void BlinkHeartbeatFreeRTOSQueues_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_FREERTOS_QUEUES_H */