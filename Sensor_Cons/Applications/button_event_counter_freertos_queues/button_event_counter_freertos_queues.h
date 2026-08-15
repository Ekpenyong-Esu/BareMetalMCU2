/**
 * @file    button_event_counter_freertos_queues.h
 * @brief   Application 2 - Button Event Counter on FreeRTOS (queues).
 */

#ifndef BUTTON_EVENT_COUNTER_FREERTOS_QUEUES_H
#define BUTTON_EVENT_COUNTER_FREERTOS_QUEUES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the button event counter on FreeRTOS (producer/consumer queue).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; never returns.
 */
void ButtonEventCounterFreeRTOSQueues_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_EVENT_COUNTER_FREERTOS_QUEUES_H */
