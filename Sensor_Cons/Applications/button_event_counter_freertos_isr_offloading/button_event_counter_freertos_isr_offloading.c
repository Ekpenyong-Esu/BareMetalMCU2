/**
 * @file    button_event_counter_freertos_isr_offloading.c
 * @brief   Application 2 - Button Event Counter on FreeRTOS (ISR offloading).
 *
 * Deferred interrupt handling: the button's EXTI0 interrupt does the absolute
 * minimum (clear the flag, wake the button task) and ALL real work happens in
 * tasks.
 *
 *   - EXTI0_IRQHandler (stm32f4xx_it.c) forwards to ButtonCounter_EXTI_IRQHandler()
 *   - ButtonCounter_EXTI_IRQHandler() (this module, the ISR):
 *       clears the EXTI flag and wakes the button task - nothing else
 *   - button task: wakes, debounces, detects press / long-press, and sends
 *       the events into a queue
 *   - counter task: receives the events, updates the count, refreshes the LEDs
 *
 * Flow:  button edge -> EXTI0 ISR -> xTaskNotifyFromISR -> button task
 *        -> xQueueSend(event) -> queue -> counter task -> count + LEDs
 *
 * Layering:
 *   main.c                                          -> chooses the application
 *   button_event_counter_freertos_isr_offloading    -> composition + the ISR
 *   press_detector                                  -> press / long-press detection
 *   press_counter                                   -> press counting
 *   counter_display                                 -> LED output
 *   board.h                                         -> which pins the button and LEDs are wired to
 */

#include "button_event_counter_freertos_isr_offloading.h"

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

#include "stm32f4xx_hal.h"

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
static TaskHandle_t       s_buttonTask;      /* woken by the EXTI0 ISR */
static PressDetector_t    s_pressDetector;
static PressCounter_t     s_pressCounter;
static CounterDisplay_t   s_display;

/* The ISR: deferred interrupt handling --------------------------------------*/

/**
 * @brief  EXTI0 interrupt for the user button. This is the ISR, so it must
 *         stay short: it only clears the flag and wakes the button task.
 * @note   Called from EXTI0_IRQHandler() in stm32f4xx_it.c. All debounce and
 *         event work happens in the button task, not here.
 */
void ButtonCounter_EXTI_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(BOARD_BUTTON_PIN) == RESET) {
        return;
    }
    __HAL_GPIO_EXTI_CLEAR_IT(BOARD_BUTTON_PIN);

    /* The task may not exist yet if the button is pressed during init. */
    if (s_buttonTask == NULL) {
        return;
    }

    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(s_buttonTask, 0u, eNoAction, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

/* Button task: woken by the ISR, does the real work -------------------------*/
static void ButtonTask(void *arg)
{
    (void)arg;

    for (;;) {
        /* 1. Block until the ISR reports a button edge. */
        xTaskNotifyWait(0u, 0u, NULL, portMAX_DELAY);

        /* 2. Debounce: keep sampling until the driver confirms the edge. The
              driver only latches once the level has been stable for a full
              debounce window, and bouncing restarts that window, so sampling
              for exactly BUTTON_DEBOUNCE_DEFAULT would always stop one tick
              too early and lose the press. */
        const TickType_t settleEnd =
            xTaskGetTickCount() + pdMS_TO_TICKS(2u * BUTTON_DEBOUNCE_DEFAULT);
        do {
            PressDetector_Update(&s_pressDetector);

            /* 3. Report the press edge as soon as it is confirmed. */
            if (PressDetector_WasPressed(&s_pressDetector)) {
                ButtonEvent_t event = EVENT_PRESS;
                xQueueSend(s_eventQueue, &event, 0u);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1u));
        } while (xTaskGetTickCount() < settleEnd);

        /* 4. While the button is held, keep sampling so a long-press can
              be detected. */
        while (PressDetector_IsPressed(&s_pressDetector)) {
            PressDetector_Update(&s_pressDetector);
            if (PressDetector_WasLongPressed(&s_pressDetector)) {
                ButtonEvent_t event = EVENT_LONG_PRESS;
                xQueueSend(s_eventQueue, &event, 0u);
            }
            vTaskDelay(pdMS_TO_TICKS(1u));
        }
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
                log_debug("ISR_OFFLOADING: press -> count=%lu",
                          (unsigned long)PressCounter_GetCount(&s_pressCounter));
            } else if (event == EVENT_LONG_PRESS) {
                PressCounter_Reset(&s_pressCounter);
                log_debug("ISR_OFFLOADING: long-press -> reset");
            }
        }

        /* 3. Refresh the LEDs from the current state. */
        const bool pressed = PressDetector_IsPressed(&s_pressDetector);
        const uint32_t count = PressCounter_GetCount(&s_pressCounter);
        CounterDisplay_Show(&s_display, pressed, count);
    }
}

/* ------------------------------------------------------------------------- */
void ButtonEventCounterFreeRTOSIsrOffloading_Run(void)
{
    /* The on-board user button is active HIGH and uses the EXTI0 interrupt. */
    const ButtonConfig_t buttonConfig = {
        .port = BOARD_BUTTON_PORT,
        .pin = BOARD_BUTTON_PIN,
        .activeLow = BOARD_BUTTON_ACTIVE_LOW,
        .debounceMs = BUTTON_DEBOUNCE_DEFAULT,
        .enableInterrupt = true
    };

    if (!PressDetector_Init(&s_pressDetector, &buttonConfig)) {
        log_error("ISR_OFFLOADING: PressDetector_Init failed");
        Error_Handler();
    }
    log_debug("ISR_OFFLOADING: press detector ready");

    if (!PressCounter_Init(&s_pressCounter)) {
        log_error("ISR_OFFLOADING: PressCounter_Init failed");
        Error_Handler();
    }
    log_debug("ISR_OFFLOADING: press counter ready");

    if (!CounterDisplay_Init(&s_display)) {
        log_error("ISR_OFFLOADING: CounterDisplay_Init failed");
        Error_Handler();
    }
    log_debug("ISR_OFFLOADING: counter display ready");

    /* The queue that connects the button task to the counter task. */
    s_eventQueue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(ButtonEvent_t));
    if (s_eventQueue == NULL) {
        log_error("ISR_OFFLOADING: xQueueCreate failed");
        Error_Handler();
    }
    log_debug("ISR_OFFLOADING: event queue created (depth %u)", EVENT_QUEUE_LENGTH);

    /* Button task: woken by the EXTI0 ISR, does the debounce and event work. */
    if (xTaskCreate(ButtonTask, "button", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_BUTTON, &s_buttonTask) != pdPASS) {
        log_error("ISR_OFFLOADING: failed to create button task");
        Error_Handler();
    }
    log_debug("ISR_OFFLOADING: created button task");

    /* Counter task: receives events, updates the count and the LEDs. */
    if (xTaskCreate(CounterTask, "counter", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_COUNTER, NULL) != pdPASS) {
        log_error("ISR_OFFLOADING: failed to create counter task");
        Error_Handler();
    }
    log_debug("ISR_OFFLOADING: created counter task");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("ISR_OFFLOADING: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}
