/**
 * @file    button_event_counter_freertos_isr_offloading.h
 * @brief   Application 2 - Button Event Counter on FreeRTOS (ISR offloading).
 */

#ifndef BUTTON_EVENT_COUNTER_FREERTOS_ISR_OFFLOADING_H
#define BUTTON_EVENT_COUNTER_FREERTOS_ISR_OFFLOADING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the button event counter on FreeRTOS (ISR offloading).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; never returns.
 */
void ButtonEventCounterFreeRTOSIsrOffloading_Run(void);

/**
 * @brief   EXTI0 interrupt for the user button (deferred interrupt handling).
 * @note    Called from EXTI0_IRQHandler() in stm32f4xx_it.c. Kept short: it
 *          only clears the flag and wakes the button task.
 */
void ButtonCounter_EXTI_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_EVENT_COUNTER_FREERTOS_ISR_OFFLOADING_H */