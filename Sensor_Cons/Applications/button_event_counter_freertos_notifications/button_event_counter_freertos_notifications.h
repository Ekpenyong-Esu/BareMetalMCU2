/**
 * @file    button_event_counter_freertos_notifications.h
 * @brief   Application 2 - Button Event Counter on FreeRTOS (notifications).
 */

#ifndef BUTTON_EVENT_COUNTER_FREERTOS_NOTIFICATIONS_H
#define BUTTON_EVENT_COUNTER_FREERTOS_NOTIFICATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the button event counter on FreeRTOS (task notifications).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; never returns.
 */
void ButtonEventCounterFreeRTOSNotifications_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_EVENT_COUNTER_FREERTOS_NOTIFICATIONS_H */