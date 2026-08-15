/**
 * @file    button_event_counter_freertos_mutexes.h
 * @brief   Application 2 - Button Event Counter on FreeRTOS (mutexes).
 */

#ifndef BUTTON_EVENT_COUNTER_FREERTOS_MUTEXES_H
#define BUTTON_EVENT_COUNTER_FREERTOS_MUTEXES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the button event counter on FreeRTOS (mutexes).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; never returns.
 */
void ButtonEventCounterFreeRTOSMutexes_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_EVENT_COUNTER_FREERTOS_MUTEXES_H */