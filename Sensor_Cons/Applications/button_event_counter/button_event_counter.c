/**
 * @file    button_event_counter.c
 * @brief   Application 2 - Button Event Counter.
 *
 * Debounced press / long-press detection with a press counter shown on LEDs.
 *
 * Single responsibility: this module only composes the pieces. Each piece owns
 * exactly one job:
 *   - press_detector  : turns raw button input into press / long-press events
 *   - press_counter   : counts presses
 *   - counter_display : shows button state and count on the LEDs
 *
 * Flow:  button -> press_detector -> press_counter -> counter_display -> LEDs
 *
 * Layering:
 *   main.c                    -> chooses and runs the application
 *   button_event_counter      -> composition only (this module)
 *   press_detector            -> press / long-press detection
 *   press_counter             -> press counting
 *   counter_display           -> LED output
 *   board.h                   -> which pins the button and LEDs are wired to
 */

#include "button_event_counter.h"

#include "counter_display.h"
#include "press_counter.h"
#include "press_detector.h"
#include "sys.h"

#include "board.h"
#include "button.h"
#include "log.h"

/* Handles ------------------------------------------------------------------*/
static PressDetector_t   s_pressDetector;
static PressCounter_t    s_pressCounter;
static CounterDisplay_t  s_display;

/* Initialise every piece before the super-loop starts -----------------------*/
static bool ButtonEventCounter_Init(void)
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
        log_error("BUTTON_COUNTER: PressDetector_Init failed");
        return false;
    }
    log_debug("BUTTON_COUNTER: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("BUTTON_COUNTER: PressCounter_Init failed");
        return false;
    }
    log_debug("BUTTON_COUNTER: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("BUTTON_COUNTER: CounterDisplay_Init failed");
        return false;
    }
    log_debug("BUTTON_COUNTER: counter display ready");

    return true;
}

/* One super-loop iteration --------------------------------------------------*/
static void ButtonEventCounter_Task(void)
{
    /* 1. Turn raw button input into events. */
    PressDetector_Update(&s_pressDetector);

    /* 2. React to the events: count presses, reset on a long-press. */
    if (PressDetector_WasPressed(&s_pressDetector)) {
        PressCounter_Increment(&s_pressCounter);
        log_debug("BUTTON_COUNTER: press -> count=%lu",
                  (unsigned long)PressCounter_GetCount(&s_pressCounter));
    }
    if (PressDetector_WasLongPressed(&s_pressDetector)) {
        PressCounter_Reset(&s_pressCounter);
        log_debug("BUTTON_COUNTER: long-press -> reset");
    }

    /* 3. Show the result on the LEDs. */
    const bool pressed = PressDetector_IsPressed(&s_pressDetector);
    const uint32_t count = PressCounter_GetCount(&s_pressCounter);
    CounterDisplay_Show(&s_display, pressed, count);
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounter_Run(void)
{
    if (!ButtonEventCounter_Init()) {
        Error_Handler();
    }

    log_debug("BUTTON_COUNTER: starting super-loop");
    for (;;) {
        ButtonEventCounter_Task();
    }
}
