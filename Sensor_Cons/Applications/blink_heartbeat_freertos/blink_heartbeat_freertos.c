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
 * HAL_GetTick() keeps running on TIM6, so the behaviours' ms-based timing is
 * unchanged; FreeRTOS owns SysTick. All four tasks share one task body
 * (BehaviourTaskEntry); the only thing that differs per task is which
 * behaviour it ticks and how often, so that is passed in as task parameters.
 * Tasks are created with the raw FreeRTOS API (xTaskCreate / vTaskDelayUntil /
 * vTaskStartScheduler) - no CMSIS-RTOS wrapper.
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

/* Task cadence (ms) ---------------------------------------------------------*/
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

/* Task parameters -----------------------------------------------------------*/
typedef struct {
    void (*behaviour)(uint32_t nowMs);  /* behaviour to tick */
    uint32_t periodMs;                  /* cadence in milliseconds */
} TaskParams;

/* One task per behaviour, each with its own cadence. */
static TaskParams s_ledHeartbeatParams = { LedHeartbeat_Task,   TASK_PERIOD_HEARTBEAT_MS };
static TaskParams s_blinkSteadyParams  = { BlinkSteady_Task,    TASK_PERIOD_STEADY_MS };
static TaskParams s_pwmSwParams        = { PwmLedSoftware_Task, TASK_PERIOD_PWM_SW_MS };
static TaskParams s_pwmHwParams        = { PwmLedHardware_Task, TASK_PERIOD_PWM_HW_MS };

/* -------------------------------------------------------------------------- */
/**
 * @brief  Shared task body: tick one behaviour at a fixed cadence.
 * @param  arg Pointer to a TaskParams describing the behaviour and its period.
 */
static void BehaviourTaskEntry(void *arg)
{
    TaskParams *params = (TaskParams *)arg;
    TickType_t lastWake = xTaskGetTickCount();
    TickType_t periodTicks = pdMS_TO_TICKS(params->periodMs);

    for (;;) {
        uint32_t nowMs = HAL_GetTick();
        params->behaviour(nowMs);
        vTaskDelayUntil(&lastWake, periodTicks);
    }
}

/* -------------------------------------------------------------------------- */
/**
 * @brief  Create one behaviour task; traps a failed allocation.
 */
static void CreateTask(TaskFunction_t taskFn, const char *name, TaskParams *params, UBaseType_t priority)
{
    if (xTaskCreate(taskFn, name, TASK_STACK_SIZE_WORDS, params, priority, NULL) != pdPASS) {
        Error_Handler();
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
    CreateTask(BehaviourTaskEntry, "led_hb",   &s_ledHeartbeatParams, TASK_PRIO_HEARTBEAT);
    CreateTask(BehaviourTaskEntry, "blink_st", &s_blinkSteadyParams,  TASK_PRIO_STEADY);
    CreateTask(BehaviourTaskEntry, "pwm_sw",   &s_pwmSwParams,        TASK_PRIO_PWM_SW);
    CreateTask(BehaviourTaskEntry, "pwm_hw",   &s_pwmHwParams,        TASK_PRIO_PWM_HW);

    /* Hand control to the scheduler; only returns on a fatal error. */
    vTaskStartScheduler();

    Error_Handler();
}
