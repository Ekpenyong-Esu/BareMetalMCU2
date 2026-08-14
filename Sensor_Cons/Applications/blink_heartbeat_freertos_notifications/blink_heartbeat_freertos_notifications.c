/**
 * @file    blink_heartbeat_freertos_notifications.c
 * @brief   Application 3 - Blink & Heartbeat on FreeRTOS, task notifications.
 *
 * Demonstrates task notifications - the lightweight, modern alternative to
 * semaphores. No queue, no semaphore, no mutex: each behaviour task simply
 * sets its own bit in the monitor task's notification value.
 *
 *   - 4 behaviour tasks, one per LED behaviour, each paced by vTaskDelayUntil.
 *   - After stepping its LED, each behaviour task notifies the monitor with
 *     its own bit:  xTaskNotify(s_monitorTask, NOTIFY_BIT_xxx, eSetBits)
 *   - 1 monitor task blocks on xTaskNotifyWait(). Any notification wakes it;
 *     the received bits say WHICH behaviour ran. The monitor keeps the state
 *     it reads itself, so no mutex is needed here.
 *
 * Flow per behaviour:  step LED -> notify monitor (set my bit)
 *                      -> monitor wakes, records the time, checks health.
 *
 * Layering:
 *   main.c                              -> chooses and runs the application
 *   blink_heartbeat_freertos_notifications -> composition only (this module)
 *   blink_steady / led_heartbeat        -> one LED behaviour each
 *   pwm_led_software / _hardware        -> one LED behaviour each
 *   board.h                             -> which pins the LEDs are wired to
 */

#include "blink_heartbeat_freertos_notifications.h"

#include "blink_steady.h"
#include "led_heartbeat.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"

#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

/* Cadence (ms): how often each behaviour task runs --------------------------*/
#define TASK_PERIOD_PWM_SW_MS     1u   /* finest the 1 kHz tick allows */
#define TASK_PERIOD_HEARTBEAT_MS  5u
#define TASK_PERIOD_PWM_HW_MS     5u
#define TASK_PERIOD_STEADY_MS    10u

/* Task priorities (higher number = higher priority) */
#define TASK_PRIO_PWM_SW          4    /* most timing-sensitive */
#define TASK_PRIO_HEARTBEAT       3
#define TASK_PRIO_PWM_HW          2
#define TASK_PRIO_STEADY          1
#define TASK_PRIO_MONITOR         5    /* monitor serves every behaviour */

/* Task stack size in words (1 word = 4 bytes on Cortex-M4) */
#define TASK_STACK_SIZE_WORDS   256u

/* Monitor behaviour */
#define HEALTH_TIMEOUT_MS       100u  /* heartbeat must step at least this often */

/* One notification bit per behaviour ----------------------------------------*/
#define NOTIFY_BIT_HEARTBEAT    (1u << 0)
#define NOTIFY_BIT_STEADY       (1u << 1)
#define NOTIFY_BIT_PWM_SW       (1u << 2)
#define NOTIFY_BIT_PWM_HW       (1u << 3)

/* Clear every received bit as soon as the monitor wakes. */
#define NOTIFY_CLEAR_ALL_BITS  0xFFFFFFFFu

/* Handles ------------------------------------------------------------------*/
static TaskHandle_t s_monitorTask;   /* notified by every behaviour task */

/* Behaviour tasks: step the LED, then notify the monitor --------------------*/
static void LedHeartbeat_TaskEntry(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_HEARTBEAT_MS));
        uint32_t nowMs = xTaskGetTickCount();
        LedHeartbeat_Task(nowMs);

        xTaskNotify(s_monitorTask, NOTIFY_BIT_HEARTBEAT, eSetBits);
    }
}

static void BlinkSteady_TaskEntry(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_STEADY_MS));
        uint32_t nowMs = xTaskGetTickCount();
        BlinkSteady_Task(nowMs);

        xTaskNotify(s_monitorTask, NOTIFY_BIT_STEADY, eSetBits);
    }
}

static void PwmLedSoftware_TaskEntry(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_PWM_SW_MS));
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedSoftware_Task(nowMs);

        xTaskNotify(s_monitorTask, NOTIFY_BIT_PWM_SW, eSetBits);
    }
}

static void PwmLedHardware_TaskEntry(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_PWM_HW_MS));
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedHardware_Task(nowMs);

        xTaskNotify(s_monitorTask, NOTIFY_BIT_PWM_HW, eSetBits);
    }
}

/* Monitor task: wakes on any notification, checks the heartbeat health ------*/
static void LedMonitor_TaskEntry(void *arg)
{
    (void)arg;
    uint32_t heartbeatLastMs = 0u;
    bool heartbeatWasAlive = true;

    for (;;) {
        uint32_t bits = 0u;
        xTaskNotifyWait(0u, NOTIFY_CLEAR_ALL_BITS, &bits, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();

        /* The monitor wakes for any behaviour's notification, but only the
         * heartbeat bit drives the health check below. */
        if ((bits & NOTIFY_BIT_HEARTBEAT) != 0u) {
            heartbeatLastMs = nowMs;
        }

        /* Report only when the health state changes. */
        bool heartbeatAlive = (nowMs - heartbeatLastMs) < HEALTH_TIMEOUT_MS;
        if (heartbeatAlive != heartbeatWasAlive) {
            heartbeatWasAlive = heartbeatAlive;
            if (heartbeatAlive) {
                printf("Heartbeat ALIVE\r\n");
            } else {
                printf("Heartbeat DEAD\r\n");
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
void BlinkHeartbeatFreeRTOSNotifications_Run(void)
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

    /* The monitor that receives the notification bits. Create it first so its
     * handle is available to every behaviour task below. */
    if (xTaskCreate(LedMonitor_TaskEntry, "led_monitor", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_MONITOR, &s_monitorTask) != pdPASS) {
        Error_Handler();
    }

    /* One task per behaviour; each notifies the monitor after stepping. */
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