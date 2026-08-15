/**
 * @file    button_event_counter_freertos_software_timers.h
 * @brief   Application 2 - Button Event Counter on FreeRTOS (software timers).
 */

#ifndef BUTTON_EVENT_COUNTER_FREERTOS_SOFTWARE_TIMERS_H
#define BUTTON_EVENT_COUNTER_FREERTOS_SOFTWARE_TIMERS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the button event counter on FreeRTOS (software timers).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; never returns.
 */
void ButtonEventCounterFreeRTOSSoftwareTimers_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_EVENT_COUNTER_FREERTOS_SOFTWARE_TIMERS_H */