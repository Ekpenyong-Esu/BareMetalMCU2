/**
 * @file    blink_heartbeat_freertos.c
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS (raw API, no CMSIS).
 *
 * The same four LED behaviours as blink_heartbeat, but instead of one
 * super-loop each behaviour runs in its own FreeRTOS task. The scheduler
 * gives every LED its own cadence:
 *   - pwm_led_software : 1 ms  (software PWM needs the fastest service)
 *   - led_heartbeat    : 5 ms  (lub-dub rhythm)
 *   - pwm_led_hardware : 5 ms  (timer PWM just needs the target handed over)
 *   - blink_steady     : 10 ms (plain on/off blink)
 *
 * FreeRTOS owns SysTick (1 kHz tick); the tasks read xTaskGetTickCount() for
 * their ms-based timing. Each task is self-contained and ticks one behaviour
 * at a fixed cadence. Tasks are created with the raw FreeRTOS API
 * (xTaskCreate / vTaskDelayUntil / vTaskStartScheduler) - no CMSIS-RTOS wrapper.
 *
 * Layering:
 *   main.c                          -> chooses and runs the application
 *   blink_heartbeat_freertos        -> composition only (this module)
 *   blink_steady / led_heartbeat    -> one LED behaviour each
 *   pwm_led_software / _hardware    -> one LED behaviour each
 *   led_blink / led_pattern         -> reusable timing engines (drivers)
 *   led / led_software_pwm / led_pwm_timer -> LED state and PWM output stages (drivers)
 *   board.h                         -> which pins the LEDs are wired to
 */

#include "blink_heartbeat_freertos.h"

#include "blink_steady.h"
#include "led_heartbeat.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"

#include "FreeRTOS.h"
#include "task.h"

/* Task cadence (ms) This is how often the task wakes up ---------------------------------------------------------*/
#define TASK_PERIOD_PWM_SW_MS     1u   /* finest the 1 kHz tick allows */
#define TASK_PERIOD_HEARTBEAT_MS  5u
#define TASK_PERIOD_PWM_HW_MS     5u
#define TASK_PERIOD_STEADY_MS    10u

/* Task priorities (higher number = higher priority) */
#define TASK_PRIO_PWM_SW          4    /* most timing-sensitive */
#define TASK_PRIO_HEARTBEAT       3
#define TASK_PRIO_PWM_HW          2
#define TASK_PRIO_STEADY          1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4) */
#define TASK_STACK_SIZE_WORDS   256u

/* Task entry points ---------------------------------------------------------*/
static void LedHeartbeat_TaskEntry(void *arg);
static void BlinkSteady_TaskEntry(void *arg);
static void PwmLedSoftware_TaskEntry(void *arg);
static void PwmLedHardware_TaskEntry(void *arg);

/* -------------------------------------------------------------------------- */
/**
 * @brief  Red LED lub-dub rhythm task.
 */
static void LedHeartbeat_TaskEntry(void *arg)
{
    TickType_t lastWake = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t nowMs = xTaskGetTickCount();
        LedHeartbeat_Task(nowMs);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_HEARTBEAT_MS));
    }
}

/**
 * @brief  External LED fixed-rate blink task.
 */
static void BlinkSteady_TaskEntry(void *arg)
{
    TickType_t lastWake = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t nowMs = xTaskGetTickCount();
        BlinkSteady_Task(nowMs);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_STEADY_MS));
    }
}

/**
 * @brief  Green LED software-PWM breathe task.
 * @note   Runs at the fastest cadence the 1 kHz tick allows; the software PWM
 *         carrier is bit-banged, so a slower task would show as flicker.
 */
static void PwmLedSoftware_TaskEntry(void *arg)
{
    TickType_t lastWake = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedSoftware_Task(nowMs);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_PWM_SW_MS));
    }
}

/**
 * @brief  External LED timer-PWM breathe task.
 */
static void PwmLedHardware_TaskEntry(void *arg)
{
    TickType_t lastWake = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedHardware_Task(nowMs);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_PWM_HW_MS));
    }
}

void BlinkHeartbeatFreeRTOS_Run(void)
{
    /* Start every behaviour before the scheduler runs. */
    if (!BlinkSteady_Init()) {
        Error_Handler();
    }
    if (!LedHeartbeat_Init()) {
        Error_Handler();
    }
    if (!PwmLedSoftware_Init()) {
        Error_Handler();
    }
    if (!PwmLedHardware_Init()) {
        Error_Handler();
    }

    /* One task per behaviour. Stacks come from the FreeRTOS heap (heap_4). */
    if (xTaskCreate(LedHeartbeat_TaskEntry,   "led_hb",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_HEARTBEAT, NULL) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(BlinkSteady_TaskEntry,    "blink_st", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_STEADY,    NULL) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedSoftware_TaskEntry, "pwm_sw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_SW,    NULL) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedHardware_TaskEntry, "pwm_hw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_HW,    NULL) != pdPASS) {
        Error_Handler();
    }

    /* Hand control to the scheduler; only returns on a fatal error. */
    vTaskStartScheduler();

    Error_Handler();
}
