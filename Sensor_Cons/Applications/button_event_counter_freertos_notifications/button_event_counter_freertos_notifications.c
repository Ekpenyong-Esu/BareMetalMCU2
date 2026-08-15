/**
 * @file    button_event_counter_freertos_notifications.c
 * @brief   Application 2 - Button Event Counter on FreeRTOS (notifications).
 *
 * Signalling via task notifications:
 *   - button task (producer): polls the button and notifies the counter task
 *     directly, carrying the event type as the notification value.
 *   - counter task (consumer): waits on its notification value, updates the
 *     count, and refreshes the LEDs.
 *
 * Flow:  button -> xTaskNotify(value) -> counter task -> count + LEDs
 * A task notification is the lightest FreeRTOS signal: no queue, no semaphore
 * object, just a 32-bit value stored on the receiving task itself.
 *
 * Layering:
 *   main.c                                      -> chooses and runs the application
 *   button_event_counter_freertos_notifications -> composition only (this module)
 *   press_detector                              -> press / long-press detection
 *   press_counter                               -> press counting
 *   counter_display                             -> LED output
 *   board.h                                     -> which pins the button and LEDs are wired to
 */

#include "button_event_counter_freertos_notifications.h"

#include "counter_display.h"
#include "press_counter.h"
#include "press_detector.h"
#include "sys.h"

#include "board.h"
#include "button.h"
#include "log.h"

#include "FreeRTOS.h"
#include "task.h"

/* Poll cadence (ms): how often the button task samples the button. */
#define BUTTON_POLL_MS       10u

/* Display refresh (ms): how often the counter task redraws the LEDs. */
#define DISPLAY_REFRESH_MS   10u

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_BUTTON     2
#define TASK_PRIO_COUNTER    1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS  256u

/* Notification values sent to the counter task. */
#define NOTIFY_PRESS         1u
#define NOTIFY_LONG_PRESS    2u

/* Handles ------------------------------------------------------------------*/
static TaskHandle_t       s_counterTaskHandle;
static PressDetector_t    s_pressDetector;
static PressCounter_t     s_pressCounter;
static CounterDisplay_t   s_display;

/* Producer: samples the button and notifies the counter task -----------------*/
static void ButtonTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Sample the button and update the press / long-press events. */
        PressDetector_Update(&s_pressDetector);

        /* 2. Notify the counter task, carrying the event type as the value. */
        if (PressDetector_WasPressed(&s_pressDetector)) {
            xTaskNotify(s_counterTaskHandle, NOTIFY_PRESS, eSetValueWithOverwrite);
        }
        if (PressDetector_WasLongPressed(&s_pressDetector)) {
            xTaskNotify(s_counterTaskHandle, NOTIFY_LONG_PRESS, eSetValueWithOverwrite);
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

/* Consumer: waits for notifications, updates the count and the LEDs ----------*/
static void CounterTask(void *arg)
{
    (void)arg;
    uint32_t notification = 0u;

    for (;;) {
        /* 1. Wait for a notification, but wake up anyway to refresh the LEDs. */
        if (xTaskNotifyWait(0u, 0u, &notification,
                            pdMS_TO_TICKS(DISPLAY_REFRESH_MS)) == pdTRUE) {
            /* 2. React to the notification value. */
            if (notification == NOTIFY_PRESS) {
                PressCounter_Increment(&s_pressCounter);
                log_debug("NOTIFICATIONS: press -> count=%lu",
                          (unsigned long)PressCounter_GetCount(&s_pressCounter));
            } else if (notification == NOTIFY_LONG_PRESS) {
                PressCounter_Reset(&s_pressCounter);
                log_debug("NOTIFICATIONS: long-press -> reset");
            }
        }

        /* 3. Refresh the LEDs from the current state. */
        const bool pressed = PressDetector_IsPressed(&s_pressDetector);
        const uint32_t count = PressCounter_GetCount(&s_pressCounter);
        CounterDisplay_Show(&s_display, pressed, count);
    }
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounterFreeRTOSNotifications_Run(void)
{
    /* The on-board user button is active HIGH and polled (no EXTI). */
    const ButtonConfig_t buttonConfig = {
        .port = BOARD_BUTTON_PORT,
        .pin = BOARD_BUTTON_PIN,
        .activeLow = BOARD_BUTTON_ACTIVE_LOW,
        .debounceMs = BUTTON_DEBOUNCE_DEFAULT,
        .enableInterrupt = false
    };

    if (!PressDetector_Init(&s_pressDetector, &buttonConfig)) {
        log_error("NOTIFICATIONS: PressDetector_Init failed");
        Error_Handler();
    }
    log_debug("NOTIFICATIONS: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("NOTIFICATIONS: PressCounter_Init failed");
        Error_Handler();
    }
    log_debug("NOTIFICATIONS: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("NOTIFICATIONS: CounterDisplay_Init failed");
        Error_Handler();
    }
    log_debug("NOTIFICATIONS: counter display ready");

    /* Consumer first, so the button task has a handle to notify. */
    if (xTaskCreate(CounterTask, "counter", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_COUNTER, &s_counterTaskHandle) != pdPASS) {
        log_error("NOTIFICATIONS: failed to create counter task");
        Error_Handler();
    }
    log_debug("NOTIFICATIONS: created counter task");

    /* Producer: samples the button and notifies the counter task. */
    if (xTaskCreate(ButtonTask, "button", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_BUTTON, NULL) != pdPASS) {
        log_error("NOTIFICATIONS: failed to create button task");
        Error_Handler();
    }
    log_debug("NOTIFICATIONS: created button task");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("NOTIFICATIONS: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}