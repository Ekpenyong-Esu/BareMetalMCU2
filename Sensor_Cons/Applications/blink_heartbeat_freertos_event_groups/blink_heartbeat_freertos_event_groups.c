/**
 * @file    blink_heartbeat_freertos_event_groups.c
 * @brief   Application 4 - Blink & Heartbeat on FreeRTOS, event groups.
 *
 * Demonstrates event groups - the "event-driven / blocking" pattern. A shared
 * event group holds one flag per behaviour. Software timers are the event
 * sources: each timer fires at its behaviour's cadence and sets that
 * behaviour's flag. Each behaviour task blocks on its own flag and only runs
 * when the flag is set.
 *
 *   - 4 software timers, one per LED behaviour. Each timer callback sets its
 *     behaviour's flag:  xEventGroupSetBits(s_eventGroup, EVENT_BIT_xxx)
 *   - 4 behaviour tasks, one per LED behaviour. Each blocks on its own flag:
 *         xEventGroupWaitBits(s_eventGroup, EVENT_BIT_xxx, pdTRUE, ...)
 *     and steps the LED when the flag arrives.
 *
 * Flow per behaviour:  timer fires -> set my flag -> task wakes -> step LED
 *
 * Layering:
 *   main.c                              -> chooses and runs the application
 *   blink_heartbeat_freertos_event_groups -> composition only (this module)
 *   blink_steady / led_heartbeat        -> one LED behaviour each
 *   pwm_led_software / _hardware        -> one LED behaviour each
 *   board.h                             -> which pins the LEDs are wired to
 */

#include "blink_heartbeat_freertos_event_groups.h"

#include "blink_steady.h"
#include "led_heartbeat.h"
#include "log.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "timers.h"

/* Cadence (ms): how often each behaviour's timer fires ----------------------*/
#define TIMER_PERIOD_PWM_SW_MS     1u   /* finest the 1 kHz tick allows */
#define TIMER_PERIOD_HEARTBEAT_MS  5u
#define TIMER_PERIOD_PWM_HW_MS     5u
#define TIMER_PERIOD_STEADY_MS    10u

/* Task priorities (higher number = higher priority) */
#define TASK_PRIO_PWM_SW          4    /* most timing-sensitive */
#define TASK_PRIO_HEARTBEAT       3
#define TASK_PRIO_PWM_HW          2
#define TASK_PRIO_STEADY          1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4) */
#define TASK_STACK_SIZE_WORDS   256u

/* One event flag per behaviour ----------------------------------------------*/
#define EVENT_BIT_HEARTBEAT     (1u << 0)
#define EVENT_BIT_STEADY        (1u << 1)
#define EVENT_BIT_PWM_SW        (1u << 2)
#define EVENT_BIT_PWM_HW        (1u << 3)

/* Handles ------------------------------------------------------------------*/
static EventGroupHandle_t s_eventGroup;

/* Timer callbacks: the event sources. Each sets its behaviour's flag. -------*/
static void LedHeartbeatTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xEventGroupSetBits(s_eventGroup, EVENT_BIT_HEARTBEAT);
}

static void BlinkSteadyTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xEventGroupSetBits(s_eventGroup, EVENT_BIT_STEADY);
}

static void PwmLedSoftwareTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xEventGroupSetBits(s_eventGroup, EVENT_BIT_PWM_SW);
}

static void PwmLedHardwareTimer_Callback(TimerHandle_t timer)
{
    (void)timer;
    xEventGroupSetBits(s_eventGroup, EVENT_BIT_PWM_HW);
}

/* Behaviour tasks: block on their flag, then step the LED -------------------*/
static void LedHeartbeat_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xEventGroupWaitBits(s_eventGroup, EVENT_BIT_HEARTBEAT, pdTRUE, pdFALSE, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        LedHeartbeat_Task(nowMs);
    }
}

static void BlinkSteady_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xEventGroupWaitBits(s_eventGroup, EVENT_BIT_STEADY, pdTRUE, pdFALSE, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        BlinkSteady_Task(nowMs);
    }
}

static void PwmLedSoftware_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xEventGroupWaitBits(s_eventGroup, EVENT_BIT_PWM_SW, pdTRUE, pdFALSE, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedSoftware_Task(nowMs);
    }
}

static void PwmLedHardware_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xEventGroupWaitBits(s_eventGroup, EVENT_BIT_PWM_HW, pdTRUE, pdFALSE, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedHardware_Task(nowMs);
    }
}

/* ------------------------------------------------------------------------- */
void BlinkHeartbeatFreeRTOSEventGroups_Run(void)
{
    /* Start every behaviour before the scheduler runs. */
    if (!BlinkSteady_Init()) {
        log_error("EVENT_GROUPS: BlinkSteady_Init failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: BlinkSteady_Init OK");
    if (!LedHeartbeat_Init()) {
        log_error("EVENT_GROUPS: LedHeartbeat_Init failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: LedHeartbeat_Init OK");
    if (!PwmLedSoftware_Init()) {
        log_error("EVENT_GROUPS: PwmLedSoftware_Init failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: PwmLedSoftware_Init OK");
    if (!PwmLedHardware_Init()) {
        log_error("EVENT_GROUPS: PwmLedHardware_Init failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: PwmLedHardware_Init OK");

    /* The shared event group: one flag per behaviour. */
    s_eventGroup = xEventGroupCreate();
    if (s_eventGroup == NULL) {
        log_error("EVENT_GROUPS: xEventGroupCreate failed");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: event group created");

    /* One software timer per behaviour; each sets its flag when it fires. */
    TimerHandle_t heartbeatTimer = xTimerCreate("hb_timer", pdMS_TO_TICKS(TIMER_PERIOD_HEARTBEAT_MS), pdTRUE, NULL, LedHeartbeatTimer_Callback);
    if (heartbeatTimer == NULL) {
        log_error("EVENT_GROUPS: failed to create hb_timer");
        Error_Handler();
    }
    if (xTimerStart(heartbeatTimer, 0u) != pdPASS) {
        log_error("EVENT_GROUPS: failed to start hb_timer");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: started hb_timer");

    TimerHandle_t steadyTimer = xTimerCreate("steady_timer", pdMS_TO_TICKS(TIMER_PERIOD_STEADY_MS), pdTRUE, NULL, BlinkSteadyTimer_Callback);
    if (steadyTimer == NULL) {
        log_error("EVENT_GROUPS: failed to create steady_timer");
        Error_Handler();
    }
    if (xTimerStart(steadyTimer, 0u) != pdPASS) {
        log_error("EVENT_GROUPS: failed to start steady_timer");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: started steady_timer");

    TimerHandle_t pwmSwTimer = xTimerCreate("pwm_sw_timer", pdMS_TO_TICKS(TIMER_PERIOD_PWM_SW_MS), pdTRUE, NULL, PwmLedSoftwareTimer_Callback);
    if (pwmSwTimer == NULL) {
        log_error("EVENT_GROUPS: failed to create pwm_sw_timer");
        Error_Handler();
    }
    if (xTimerStart(pwmSwTimer, 0u) != pdPASS) {
        log_error("EVENT_GROUPS: failed to start pwm_sw_timer");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: started pwm_sw_timer");

    TimerHandle_t pwmHwTimer = xTimerCreate("pwm_hw_timer", pdMS_TO_TICKS(TIMER_PERIOD_PWM_HW_MS), pdTRUE, NULL, PwmLedHardwareTimer_Callback);
    if (pwmHwTimer == NULL) {
        log_error("EVENT_GROUPS: failed to create pwm_hw_timer");
        Error_Handler();
    }
    if (xTimerStart(pwmHwTimer, 0u) != pdPASS) {
        log_error("EVENT_GROUPS: failed to start pwm_hw_timer");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: started pwm_hw_timer");

    /* One task per behaviour; each blocks on its own event flag. */
    if (xTaskCreate(LedHeartbeat_TaskEntry,   "led_hb",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_HEARTBEAT, NULL) != pdPASS) {
        log_error("EVENT_GROUPS: failed to create led_hb");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: created led_hb");
    if (xTaskCreate(BlinkSteady_TaskEntry,    "blink_st", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_STEADY,    NULL) != pdPASS) {
        log_error("EVENT_GROUPS: failed to create blink_st");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: created blink_st");
    if (xTaskCreate(PwmLedSoftware_TaskEntry, "pwm_sw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_SW,    NULL) != pdPASS) {
        log_error("EVENT_GROUPS: failed to create pwm_sw");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: created pwm_sw");
    if (xTaskCreate(PwmLedHardware_TaskEntry, "pwm_hw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_HW,    NULL) != pdPASS) {
        log_error("EVENT_GROUPS: failed to create pwm_hw");
        Error_Handler();
    }
    log_debug("EVENT_GROUPS: created pwm_hw");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("EVENT_GROUPS: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}
