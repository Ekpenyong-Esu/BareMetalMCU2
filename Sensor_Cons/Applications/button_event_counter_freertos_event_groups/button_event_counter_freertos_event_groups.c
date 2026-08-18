/**
 * @file    button_event_counter_freertos_event_groups.c
 * @brief   Application 2 - Button Event Counter on FreeRTOS (event groups).
 *
 * Signalling via ONE event group:
 *   - button task (producer): polls the button and sets a bit for each event
 *     (one bit for a press, one for a long-press).
 *   - counter task (consumer): waits on the event group, reacts to whichever
 *     bits are set, and refreshes the LEDs.
 *
 * Flow:  button -> xEventGroupSetBits -> event group -> xEventGroupWaitBits
 *        -> count + LEDs
 * An event group is a set of flags: each bit is an independent event, so the
 * consumer can wait for "any" or "all" of them and react per bit.
 *
 * Layering:
 *   main.c                                    -> chooses and runs the application
 *   button_event_counter_freertos_event_groups -> composition only (this module)
 *   press_detector                            -> press / long-press detection
 *   press_counter                             -> press counting
 *   counter_display                           -> LED output
 *   board.h                                   -> which pins the button and LEDs are wired to
 */

#include "button_event_counter_freertos_event_groups.h"

#include "counter_display.h"
#include "press_counter.h"
#include "press_detector.h"
#include "sys.h"

#include "board.h"
#include "button.h"
#include "log.h"

#include "FreeRTOS.h"
#include "event_groups.h"
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

/* Event group bits: one per event type. */
#define EVENT_PRESS_BIT       (1u << 0)
#define EVENT_LONG_PRESS_BIT  (1u << 1)

/* Handles ------------------------------------------------------------------*/
static EventGroupHandle_t s_eventGroup;
static PressDetector_t    s_pressDetector;
static PressCounter_t     s_pressCounter;
static CounterDisplay_t   s_display;

/* Producer: samples the button and sets the matching event bit ----------------*/
static void ButtonTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Sample the button and update the press / long-press events. */
        PressDetector_Update(&s_pressDetector);

        /* 2. Set the bit for each event that happened. */
        if (PressDetector_WasPressed(&s_pressDetector)) {
            xEventGroupSetBits(s_eventGroup, EVENT_PRESS_BIT);
        }
        if (PressDetector_WasLongPressed(&s_pressDetector)) {
            xEventGroupSetBits(s_eventGroup, EVENT_LONG_PRESS_BIT);
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

/* Consumer: waits for event bits, updates the count and the LEDs -------------*/
static void CounterTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Wait for any event bit, but wake up anyway to refresh the LEDs. */
        const EventBits_t bits = xEventGroupWaitBits(
            s_eventGroup,
            EVENT_PRESS_BIT | EVENT_LONG_PRESS_BIT,  /* bits to wait for */
            pdTRUE,                                  /* clear them on exit */
            pdFALSE,                                 /* any bit, not all */
            pdMS_TO_TICKS(DISPLAY_REFRESH_MS));

        /* 2. React to each bit that was set. */
        if ((bits & EVENT_PRESS_BIT) != 0u) {
            PressCounter_Increment(&s_pressCounter);
            log_debug("EVENT_GROUPS: press -> count=%lu",
                      (unsigned long)PressCounter_GetCount(&s_pressCounter));
        }
        if ((bits & EVENT_LONG_PRESS_BIT) != 0u) {
            PressCounter_Reset(&s_pressCounter);
            log_debug("EVENT_GROUPS: long-press -> reset");
        }

        /* 3. Refresh the LEDs from the current state. */
        const bool pressed = PressDetector_IsPressed(&s_pressDetector);
        const uint32_t count = PressCounter_GetCount(&s_pressCounter);
        CounterDisplay_Show(&s_display, pressed, count);
    }
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounterFreeRTOSEventGroups_Run(void)
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
        log_error("EVENT_GROUPS: PressDetector_Init failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("EVENT_GROUPS: PressCounter_Init failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("EVENT_GROUPS: CounterDisplay_Init failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: counter display ready");

    /* The event group that carries the event bits. */
    s_eventGroup = xEventGroupCreate();
    if (s_eventGroup == NULL) {
        log_error("EVENT_GROUPS: xEventGroupCreate failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: event group created");

    /* Producer: samples the button and sets the event bits. */
    if (xTaskCreate(ButtonTask, "button", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_BUTTON, NULL) != pdPASS) {
        log_error("EVENT_GROUPS: failed to create button task");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: created button task");

    /* Consumer: waits for the event bits, updates the count and the LEDs. */
    if (xTaskCreate(CounterTask, "counter", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_COUNTER, NULL) != pdPASS) {
        log_error("EVENT_GROUPS: failed to create counter task");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: created counter task");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("EVENT_GROUPS: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}
