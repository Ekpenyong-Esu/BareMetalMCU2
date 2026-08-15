/**
 * @file    button_event_counter_freertos_software_timers.c
 * @brief   Application 2 - Button Event Counter on FreeRTOS (software timers).
 *
 * A FreeRTOS software timer replaces the manual long-press timing:
 *   - button task: polls the button. On a press edge it sends EVENT_PRESS and
 *     starts a one-shot long-press timer; on release it stops the timer.
 *   - long-press timer: a one-shot software timer (1000 ms). If it fires while
 *     the button is still held, its callback sends EVENT_LONG_PRESS.
 *   - counter task: receives the events, updates the count, refreshes the LEDs.
 *
 * Flow:  button -> xQueueSend(EVENT_PRESS) -> queue -> counter task
 *        button held 1000 ms -> timer callback -> xQueueSend(EVENT_LONG_PRESS)
 *
 * Layering:
 *   main.c                                          -> chooses the application
 *   button_event_counter_freertos_software_timers   -> composition only
 *   press_detector                                  -> press / release edges
 *   press_counter                                   -> press counting
 *   counter_display                                 -> LED output
 *   board.h                                         -> which pins the button and LEDs are wired to
 */

#include "button_event_counter_freertos_software_timers.h"

#include "counter_display.h"
#include "press_counter.h"
#include "press_detector.h"
#include "sys.h"

#include "board.h"
#include "button.h"
#include "log.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

/* Poll cadence (ms): how often the button task samples the button. */
#define BUTTON_POLL_MS       10u

/* Display refresh (ms): how often the counter task redraws the LEDs. */
#define DISPLAY_REFRESH_MS   10u

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_BUTTON     2
#define TASK_PRIO_COUNTER    1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS  256u

/* Queue depth: one slot per event type is plenty. */
#define EVENT_QUEUE_LENGTH   4u

/* Message carried by the queue: which event happened. */
typedef enum {
    EVENT_PRESS,
    EVENT_LONG_PRESS
} ButtonEvent_t;

/* Handles ------------------------------------------------------------------*/
static QueueHandle_t      s_eventQueue;
static TimerHandle_t      s_longPressTimer;   /* one-shot, 1000 ms */
static PressDetector_t    s_pressDetector;
static PressCounter_t     s_pressCounter;
static CounterDisplay_t   s_display;

/* Long-press timer callback: runs in the Timer Service task -----------------*/

/**
 * @brief  Fires when the button has been held for PRESS_LONG_MS.
 * @note   Runs in the Timer Service (daemon) task, so it may use the queue.
 */
static void LongPressTimer_Callback(TimerHandle_t timer)
{
    (void)timer;

    /* Only report a long-press if the button is still held. */
    if (!PressDetector_IsPressed(&s_pressDetector)) {
        return;
    }

    ButtonEvent_t event = EVENT_LONG_PRESS;
    xQueueSend(s_eventQueue, &event, 0u);
}

/* Producer: samples the button and arms / cancels the long-press timer -------*/
static void ButtonTask(void *arg)
{
    (void)arg;
    bool wasPressed = false;

    for (;;) {
        /* 1. Sample the button and update the press / release edges. */
        PressDetector_Update(&s_pressDetector);

        /* 2. On a press edge: count it and arm the long-press timer. */
        if (PressDetector_WasPressed(&s_pressDetector)) {
            ButtonEvent_t event = EVENT_PRESS;
            xQueueSend(s_eventQueue, &event, 0u);
            xTimerStart(s_longPressTimer, 0u);
            wasPressed = true;
        }

        /* 3. On release: cancel the long-press timer. */
        if (wasPressed && !PressDetector_IsPressed(&s_pressDetector)) {
            xTimerStop(s_longPressTimer, 0u);
            wasPressed = false;
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

/* Consumer: receives events, updates the count and the LEDs -----------------*/
static void CounterTask(void *arg)
{
    (void)arg;
    ButtonEvent_t event = EVENT_PRESS;

    for (;;) {
        /* 1. Wait for an event, but wake up anyway to refresh the LEDs. */
        if (xQueueReceive(s_eventQueue, &event,
                          pdMS_TO_TICKS(DISPLAY_REFRESH_MS)) == pdTRUE) {
            /* 2. React to the event: count presses, reset on a long-press. */
            if (event == EVENT_PRESS) {
                PressCounter_Increment(&s_pressCounter);
                log_debug("SOFTWARE_TIMERS: press -> count=%lu",
                          (unsigned long)PressCounter_GetCount(&s_pressCounter));
            } else if (event == EVENT_LONG_PRESS) {
                PressCounter_Reset(&s_pressCounter);
                log_debug("SOFTWARE_TIMERS: long-press -> reset");
            }
        }

        /* 3. Refresh the LEDs from the current state. */
        const bool pressed = PressDetector_IsPressed(&s_pressDetector);
        const uint32_t count = PressCounter_GetCount(&s_pressCounter);
        CounterDisplay_Show(&s_display, pressed, count);
    }
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounterFreeRTOSSoftwareTimers_Run(void)
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
        log_error("SOFTWARE_TIMERS: PressDetector_Init failed");
        Error_Handler();
    }
    log_debug("SOFTWARE_TIMERS: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("SOFTWARE_TIMERS: PressCounter_Init failed");
        Error_Handler();
    }
    log_debug("SOFTWARE_TIMERS: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("SOFTWARE_TIMERS: CounterDisplay_Init failed");
        Error_Handler();
    }
    log_debug("SOFTWARE_TIMERS: counter display ready");

    /* The queue that connects the button task to the counter task. */
    s_eventQueue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(ButtonEvent_t));
    if (s_eventQueue == NULL) {
        log_error("SOFTWARE_TIMERS: xQueueCreate failed");
        Error_Handler();
    }
    log_debug("SOFTWARE_TIMERS: event queue created (depth %u)", EVENT_QUEUE_LENGTH);

    /* One-shot long-press timer: fires after PRESS_LONG_MS of holding. */
    s_longPressTimer = xTimerCreate("long-press",
                                    pdMS_TO_TICKS(PRESS_LONG_MS),
                                    pdFALSE,          /* one-shot */
                                    (void *)0u,
                                    LongPressTimer_Callback);
    if (s_longPressTimer == NULL) {
        log_error("SOFTWARE_TIMERS: xTimerCreate failed");
        Error_Handler();
    }
    log_debug("SOFTWARE_TIMERS: long-press timer created (%lu ms)",
              (unsigned long)PRESS_LONG_MS);

    /* Producer: samples the button and arms the long-press timer. */
    if (xTaskCreate(ButtonTask, "button", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_BUTTON, NULL) != pdPASS) {
        log_error("SOFTWARE_TIMERS: failed to create button task");
        Error_Handler();
    }
    log_debug("SOFTWARE_TIMERS: created button task");

    /* Consumer: receives events, updates the count and the LEDs. */
    if (xTaskCreate(CounterTask, "counter", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_COUNTER, NULL) != pdPASS) {
        log_error("SOFTWARE_TIMERS: failed to create counter task");
        Error_Handler();
    }
    log_debug("SOFTWARE_TIMERS: created counter task");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("SOFTWARE_TIMERS: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}