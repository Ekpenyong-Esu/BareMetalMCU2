/**
 * @file    button_event_counter_freertos_mutexes.c
 * @brief   Application 2 - Button Event Counter on FreeRTOS (mutexes).
 *
 * A mutex protects the shared press counter between two tasks:
 *   - button task: polls the button. On a press it increments the counter;
 *     on a long-press it resets the counter. Both writes take the mutex.
 *   - display task: every 10 ms it reads the counter under the mutex and
 *     refreshes the LEDs.
 *
 * Flow:  button -> mutex -> counter (shared) -> mutex -> LEDs
 *
 * Layering:
 *   main.c                                  -> chooses the application
 *   button_event_counter_freertos_mutexes   -> composition only
 *   press_detector                          -> press / long-press edges
 *   press_counter                           -> press counting (shared)
 *   counter_display                         -> LED output
 *   board.h                                 -> which pins the button and LEDs are wired to
 */

#include "button_event_counter_freertos_mutexes.h"

#include "counter_display.h"
#include "press_counter.h"
#include "press_detector.h"
#include "sys.h"

#include "board.h"
#include "button.h"
#include "log.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* Poll cadence (ms): how often the button task samples the button. */
#define BUTTON_POLL_MS       10u

/* Display refresh (ms): how often the display task redraws the LEDs. */
#define DISPLAY_REFRESH_MS   10u

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_BUTTON     2
#define TASK_PRIO_DISPLAY    1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS  256u

/* Handles ------------------------------------------------------------------*/
static SemaphoreHandle_t  s_counterMutex;   /* guards the shared press counter */
static PressDetector_t    s_pressDetector;
static PressCounter_t     s_pressCounter;
static CounterDisplay_t   s_display;

/* Producer: samples the button and updates the shared counter ----------------*/
static void ButtonTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Sample the button and update the press / long-press edges. */
        PressDetector_Update(&s_pressDetector);

        /* 2. On a press: increment the shared counter (under the mutex). */
        if (PressDetector_WasPressed(&s_pressDetector)) {
            xSemaphoreTake(s_counterMutex, portMAX_DELAY);
            PressCounter_Increment(&s_pressCounter);
            const uint32_t count = PressCounter_GetCount(&s_pressCounter);
            xSemaphoreGive(s_counterMutex);
            log_debug("MUTEXES: press -> count=%lu", (unsigned long)count);
        }

        /* 3. On a long-press: reset the shared counter (under the mutex). */
        if (PressDetector_WasLongPressed(&s_pressDetector)) {
            xSemaphoreTake(s_counterMutex, portMAX_DELAY);
            PressCounter_Reset(&s_pressCounter);
            xSemaphoreGive(s_counterMutex);
            log_debug("MUTEXES: long-press -> reset");
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

/* Consumer: reads the shared counter and refreshes the LEDs ------------------*/
static void DisplayTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Read the shared counter under the mutex. */
        xSemaphoreTake(s_counterMutex, portMAX_DELAY);
        const uint32_t count = PressCounter_GetCount(&s_pressCounter);
        xSemaphoreGive(s_counterMutex);

        /* 2. Refresh the LEDs from the current state. */
        const bool pressed = PressDetector_IsPressed(&s_pressDetector);
        CounterDisplay_Show(&s_display, pressed, count);

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_MS));
    }
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounterFreeRTOSMutexes_Run(void)
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
        log_error("MUTEXES: PressDetector_Init failed");
        Error_Handler();
    }
    log_debug("MUTEXES: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("MUTEXES: PressCounter_Init failed");
        Error_Handler();
    }
    log_debug("MUTEXES: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("MUTEXES: CounterDisplay_Init failed");
        Error_Handler();
    }
    log_debug("MUTEXES: counter display ready");

    /* The mutex that guards the shared press counter. */
    s_counterMutex = xSemaphoreCreateMutex();
    if (s_counterMutex == NULL) {
        log_error("MUTEXES: xSemaphoreCreateMutex failed");
        Error_Handler();
    }
    log_debug("MUTEXES: counter mutex created");

    /* Producer: samples the button and updates the shared counter. */
    if (xTaskCreate(ButtonTask, "button", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_BUTTON, NULL) != pdPASS) {
        log_error("MUTEXES: failed to create button task");
        Error_Handler();
    }
    log_debug("MUTEXES: created button task");

    /* Consumer: reads the shared counter and refreshes the LEDs. */
    if (xTaskCreate(DisplayTask, "display", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_DISPLAY, NULL) != pdPASS) {
        log_error("MUTEXES: failed to create display task");
        Error_Handler();
    }
    log_debug("MUTEXES: created display task");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("MUTEXES: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}
