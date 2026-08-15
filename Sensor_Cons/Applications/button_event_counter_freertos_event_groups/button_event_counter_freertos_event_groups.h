/**
 * @file    button_event_counter_freertos_event_groups.h
 * @brief   Application 2 - Button Event Counter on FreeRTOS (event groups).
 */

#ifndef BUTTON_EVENT_COUNTER_FREERTOS_EVENT_GROUPS_H
#define BUTTON_EVENT_COUNTER_FREERTOS_EVENT_GROUPS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the button event counter on FreeRTOS (event groups).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; never returns.
 */
void ButtonEventCounterFreeRTOSEventGroups_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_EVENT_COUNTER_FREERTOS_EVENT_GROUPS_H */