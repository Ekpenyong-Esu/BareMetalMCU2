/**
 * @file    blink_heartbeat_freertos_event_driven.c
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS, event-driven.
 *
 * The same four LED behaviours as blink_heartbeat, each in its own FreeRTOS
 * task. Unlike the periodic version, the tasks never poll or delay: each task
 * BLOCKS waiting for an event, and a FreeRTOS software timer fires that event
 * at the behaviour's cadence:
 *   - pwm_led_software : 1 ms  (software PWM needs the fastest service)
 *   - led_heartbeat    : 5 ms  (lub-dub rhythm)
 *   - pwm_led_hardware : 5 ms  (timer PWM just needs the target handed over)
 *   - blink_steady     : 10 ms (plain on/off blink)
 *
 * Flow per behaviour:  timer fires -> xTaskNotifyGive(task) ->
 * ulTaskNotifyTake() wakes the task -> one behaviour step -> block again.
 * The CPU is never busy-waiting; tasks sleep until an event arrives.
 *
 * Layering:
 *   main.c                          -> chooses and runs the application
 *   blink_heartbeat_freertos_event_driven -> composition only (this module)
 *   blink_steady / led_heartbeat    -> one LED behaviour each
 *   pwm_led_software / _hardware    -> one LED behaviour each
 *   board.h                         -> which pins the LEDs are wired to
 */

#include "blink_heartbeat_freertos_event_driven.h"

#include "blink_steady.h"
#include "led_heartbeat.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Event cadence (ms): how often each timer fires ----------------------------*/
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

/* Handles ---------------------------------------------------------------- */
static TaskHandle_t  s_ledHeartbeatTask;
static TimerHandle_t s_ledHeartbeatTimer;
static TaskHandle_t  s_blinkSteadyTask;
static TimerHandle_t s_blinkSteadyTimer;
static TaskHandle_t  s_pwmSwTask;
static TimerHandle_t s_pwmSwTimer;
static TaskHandle_t  s_pwmHwTask;
static TimerHandle_t s_pwmHwTimer;

/* ------------------------------------------------------------------------- */
/* Timer callbacks: each one wakes exactly one task ------------------------- */
static void LedHeartbeatTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xTaskNotifyGive(s_ledHeartbeatTask);
}

static void BlinkSteadyTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xTaskNotifyGive(s_blinkSteadyTask);
}

static void PwmLedSoftwareTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xTaskNotifyGive(s_pwmSwTask);
}

static void PwmLedHardwareTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xTaskNotifyGive(s_pwmHwTask);
}

/* Task entry points -------------------------------------------------------- */
static void LedHeartbeat_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* wait for the timer event */
        uint32_t nowMs = xTaskGetTickCount();
        LedHeartbeat_Task(nowMs);
    }
}

static void BlinkSteady_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* wait for the timer event */
        uint32_t nowMs = xTaskGetTickCount();
        BlinkSteady_Task(nowMs);
    }
}

static void PwmLedSoftware_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* wait for the timer event */
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedSoftware_Task(nowMs);
    }
}

static void PwmLedHardware_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* wait for the timer event */
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedHardware_Task(nowMs);
    }
}

/* ------------------------------------------------------------------------- */
void BlinkHeartbeatFreeRTOSEventDriven_Run(void)
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
    if (xTaskCreate(LedHeartbeat_TaskEntry,   "led_hb",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_HEARTBEAT, &s_ledHeartbeatTask) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(BlinkSteady_TaskEntry,    "blink_st", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_STEADY,    &s_blinkSteadyTask) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedSoftware_TaskEntry, "pwm_sw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_SW,    &s_pwmSwTask) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedHardware_TaskEntry, "pwm_hw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_HW,    &s_pwmHwTask) != pdPASS) {
        Error_Handler();
    }

    /* One software timer per behaviour: its callback wakes its own task. */
    s_ledHeartbeatTimer = xTimerCreate("led_hb_t", pdMS_TO_TICKS(TASK_PERIOD_HEARTBEAT_MS), pdTRUE, NULL, LedHeartbeatTimer_Callback);
    if (s_ledHeartbeatTimer == NULL) {
        Error_Handler();
    }
    if (xTimerStart(s_ledHeartbeatTimer, 0) != pdPASS) {
        Error_Handler();
    }

    s_blinkSteadyTimer = xTimerCreate("blink_st_t", pdMS_TO_TICKS(TASK_PERIOD_STEADY_MS), pdTRUE, NULL, BlinkSteadyTimer_Callback);
    if (s_blinkSteadyTimer == NULL) {
        Error_Handler();
    }
    if (xTimerStart(s_blinkSteadyTimer, 0) != pdPASS) {
        Error_Handler();
    }

    s_pwmSwTimer = xTimerCreate("pwm_sw_t", pdMS_TO_TICKS(TASK_PERIOD_PWM_SW_MS), pdTRUE, NULL, PwmLedSoftwareTimer_Callback);
    if (s_pwmSwTimer == NULL) {
        Error_Handler();
    }
    if (xTimerStart(s_pwmSwTimer, 0) != pdPASS) {
        Error_Handler();
    }

    s_pwmHwTimer = xTimerCreate("pwm_hw_t", pdMS_TO_TICKS(TASK_PERIOD_PWM_HW_MS), pdTRUE, NULL, PwmLedHardwareTimer_Callback);
    if (s_pwmHwTimer == NULL) {
        Error_Handler();
    }
    if (xTimerStart(s_pwmHwTimer, 0) != pdPASS) {
        Error_Handler();
    }

    /* Hand control to the scheduler; only returns on a fatal error. */
    vTaskStartScheduler();

    Error_Handler();
}
