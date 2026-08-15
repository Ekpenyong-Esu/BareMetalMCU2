/**
 * @file    button_event_counter_freertos_semaphores.c
 * @brief   Application 2 - Button Event Counter on FreeRTOS (semaphores).
 *
 * Signalling via TWO binary semaphores:
 *   - button task (producer): polls the button and gives a semaphore for each
 *     event (one for a press, one for a long-press).
 *   - counter task (consumer): takes the semaphores to wake up, updates the
 *     count, and refreshes the LEDs.
 *
 * Flow:  button -> xSemaphoreGive -> semaphore -> xSemaphoreTake -> count + LEDs
 * A binary semaphore is a lightweight "something happened" signal: it carries
 * no data, only the fact that an event occurred.
 *
 * Layering:
 *   main.c                                    -> chooses and runs the application
 *   button_event_counter_freertos_semaphores  -> composition only (this module)
 *   press_detector                            -> press / long-press detection
 *   press_counter                             -> press counting
 *   counter_display                           -> LED output
 *   board.h                                   -> which pins the button and LEDs are wired to
 */

#include "button_event_counter_freertos_semaphores.h"

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

/* Display refresh (ms): how often the counter task redraws the LEDs. */
#define DISPLAY_REFRESH_MS   10u

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_BUTTON     2
#define TASK_PRIO_COUNTER    1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS  256u

/* Handles ------------------------------------------------------------------*/
static SemaphoreHandle_t  s_pressSemaphore;      /* given on a press */
static SemaphoreHandle_t  s_longPressSemaphore;  /* given on a long-press */
static PressDetector_t    s_pressDetector;
static PressCounter_t     s_pressCounter;
static CounterDisplay_t   s_display;

/* Producer: samples the button and gives a semaphore per event ---------------*/
static void ButtonTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Sample the button and update the press / long-press events. */
        PressDetector_Update(&s_pressDetector);

        /* 2. Signal each event that happened with its own semaphore. */
        if (PressDetector_WasPressed(&s_pressDetector)) {
            xSemaphoreGive(s_pressSemaphore);
        }
        if (PressDetector_WasLongPressed(&s_pressDetector)) {
            xSemaphoreGive(s_longPressSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

/* Consumer: takes the semaphores, updates the count and the LEDs -------------*/
static void CounterTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Wait for a press, but wake up anyway to refresh the LEDs. */
        if (xSemaphoreTake(s_pressSemaphore,
                           pdMS_TO_TICKS(DISPLAY_REFRESH_MS)) == pdTRUE) {
            PressCounter_Increment(&s_pressCounter);
            log_debug("SEMAPHORES: press -> count=%lu",
                      (unsigned long)PressCounter_GetCount(&s_pressCounter));
        }

        /* 2. A long-press is signalled separately; check it without waiting. */
        if (xSemaphoreTake(s_longPressSemaphore, 0u) == pdTRUE) {
            PressCounter_Reset(&s_pressCounter);
            log_debug("SEMAPHORES: long-press -> reset");
        }

        /* 3. Refresh the LEDs from the current state. */
        const bool pressed = PressDetector_IsPressed(&s_pressDetector);
        const uint32_t count = PressCounter_GetCount(&s_pressCounter);
        CounterDisplay_Show(&s_display, pressed, count);
    }
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounterFreeRTOSSemaphores_Run(void)
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
        log_error("SEMAPHORES: PressDetector_Init failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("SEMAPHORES: PressCounter_Init failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("SEMAPHORES: CounterDisplay_Init failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: counter display ready");

    /* One binary semaphore per event type. */
    s_pressSemaphore = xSemaphoreCreateBinary();
    if (s_pressSemaphore == NULL) {
        log_error("SEMAPHORES: xSemaphoreCreateBinary (press) failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: press semaphore created");

    s_longPressSemaphore = xSemaphoreCreateBinary();
    if (s_longPressSemaphore == NULL) {
        log_error("SEMAPHORES: xSemaphoreCreateBinary (long-press) failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: long-press semaphore created");

    /* Producer: samples the button and gives the semaphores. */
    if (xTaskCreate(ButtonTask, "button", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_BUTTON, NULL) != pdPASS) {
        log_error("SEMAPHORES: failed to create button task");
        Error_Handler();
    }
    log_debug("SEMAPHORES: created button task");

    /* Consumer: takes the semaphores, updates the count and the LEDs. */
    if (xTaskCreate(CounterTask, "counter", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_COUNTER, NULL) != pdPASS) {
        log_error("SEMAPHORES: failed to create counter task");
        Error_Handler();
    }
    log_debug("SEMAPHORES: created counter task");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("SEMAPHORES: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}