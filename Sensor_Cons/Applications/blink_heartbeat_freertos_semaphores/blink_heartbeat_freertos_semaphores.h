/**
 * @file    blink_heartbeat_freertos_semaphores.h
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS, synchronised with a
 *          mutex (shared state) and a binary semaphore (change signal).
 */

#ifndef BLINK_HEARTBEAT_FREERTOS_SEMAPHORES_H
#define BLINK_HEARTBEAT_FREERTOS_SEMAPHORES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise every behaviour, create the tasks + synchronisation
 *         objects, and start the FreeRTOS scheduler.
 * @note   Requires the system clock and time bases to be running (SYS_Init).
 *         Does not return on success; calls Error_Handler() on failure.
 */
void BlinkHeartbeatFreeRTOSSemaphores_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_HEARTBEAT_FREERTOS_SEMAPHORES_H */