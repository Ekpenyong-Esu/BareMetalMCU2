/**
 * @file    button_event_counter_freertos_queues.c
 * @brief   Application 2 - Button Event Counter on FreeRTOS (queues).
 *
 * Producer/consumer via ONE FreeRTOS queue:
 *   - button task (producer): polls the button and sends PRESS / LONG_PRESS
 *     events into the queue.
 *   - counter task (consumer): blocks on the queue, updates the count, and
 *     refreshes the LEDs.
 *
 * Flow:  button -> xQueueSend(event) -> queue -> xQueueReceive -> count + LEDs
 * The queue decouples the two tasks: the button task never waits for the LED
 * work, and the counter task never decides when the button is sampled.
 *
 * Layering:
 *   main.c                                  -> chooses and runs the application
 *   button_event_counter_freertos_queues    -> composition only (this module)
 *   press_detector                          -> press / long-press detection
 *   press_counter                           -> press counting
 *   counter_display                         -> LED output
 *   board.h                                 -> which pins the button and LEDs are wired to
 */

#include "button_event_counter_freertos_queues.h"

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
static PressDetector_t    s_pressDetector;
static PressCounter_t     s_pressCounter;
static CounterDisplay_t   s_display;

/* Producer: samples the button and sends events -----------------------------*/
static void ButtonTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Sample the button and update the press / long-press events. */
        PressDetector_Update(&s_pressDetector);

        /* 2. Send each event that happened into the queue. */
        ButtonEvent_t event = EVENT_PRESS;
        if (PressDetector_WasPressed(&s_pressDetector)) {
            event = EVENT_PRESS;
            xQueueSend(s_eventQueue, &event, 0u);
        }
        if (PressDetector_WasLongPressed(&s_pressDetector)) {
            event = EVENT_LONG_PRESS;
            xQueueSend(s_eventQueue, &event, 0u);
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
                log_debug("QUEUES: press -> count=%lu",
                          (unsigned long)PressCounter_GetCount(&s_pressCounter));
            } else if (event == EVENT_LONG_PRESS) {
                PressCounter_Reset(&s_pressCounter);
                log_debug("QUEUES: long-press -> reset");
            }
        }

        /* 3. Refresh the LEDs from the current state. */
        const bool pressed = PressDetector_IsPressed(&s_pressDetector);
        const uint32_t count = PressCounter_GetCount(&s_pressCounter);
        CounterDisplay_Show(&s_display, pressed, count);
    }
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounterFreeRTOSQueues_Run(void)
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
        log_error("QUEUES: PressDetector_Init failed");
        Error_Handler();
    }
    log_debug("QUEUES: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("QUEUES: PressCounter_Init failed");
        Error_Handler();
    }
    log_debug("QUEUES: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("QUEUES: CounterDisplay_Init failed");
        Error_Handler();
    }
    log_debug("QUEUES: counter display ready");

    /* The queue that connects the button task to the counter task. */
    s_eventQueue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(ButtonEvent_t));
    if (s_eventQueue == NULL) {
        log_error("QUEUES: xQueueCreate failed");
        Error_Handler();
    }
    log_debug("QUEUES: event queue created (depth %u)", EVENT_QUEUE_LENGTH);

    /* Producer: samples the button and sends events. */
    if (xTaskCreate(ButtonTask, "button", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_BUTTON, NULL) != pdPASS) {
        log_error("QUEUES: failed to create button task");
        Error_Handler();
    }
    log_debug("QUEUES: created button task");

    /* Consumer: receives events, updates the count and the LEDs. */
    if (xTaskCreate(CounterTask, "counter", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_COUNTER, NULL) != pdPASS) {
        log_error("QUEUES: failed to create counter task");
        Error_Handler();
    }
    log_debug("QUEUES: created counter task");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("QUEUES: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}
