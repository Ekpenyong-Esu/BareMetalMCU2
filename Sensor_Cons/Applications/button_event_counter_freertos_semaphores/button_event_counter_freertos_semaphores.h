/**
 * @file    button_event_counter_freertos_semaphores.h
 * @brief   Application 2 - Button Event Counter on FreeRTOS (semaphores).
 */

#ifndef BUTTON_EVENT_COUNTER_FREERTOS_SEMAPHORES_H
#define BUTTON_EVENT_COUNTER_FREERTOS_SEMAPHORES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the button event counter on FreeRTOS (binary semaphores).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; never returns.
 */
void ButtonEventCounterFreeRTOSSemaphores_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_EVENT_COUNTER_FREERTOS_SEMAPHORES_H */