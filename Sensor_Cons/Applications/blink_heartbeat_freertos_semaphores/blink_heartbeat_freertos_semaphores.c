/**
 * @file    blink_heartbeat_freertos_semaphores.c
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS, synchronised.
 *
 * Demonstrates the two classic synchronisation primitives:
 *   - A MUTEX protects a shared LedActivity_t struct. Every behaviour task
 *     writes its own field under the mutex; the monitor task reads the whole
 *     struct under the mutex. No two tasks ever touch it at the same time.
 *   - A BINARY SEMAPHORE signals "activity changed". Each behaviour task
 *     gives it after updating the shared struct; the monitor task blocks on
 *     it, so it only wakes when there is something new to read.
 *
 * Flow per behaviour:  step LED -> lock mutex -> record activity -> unlock
 *                      -> give semaphore -> monitor wakes and reads.
 *
 * Layering:
 *   main.c                            -> chooses and runs the application
 *   blink_heartbeat_freertos_semaphores -> composition only (this module)
 *   blink_steady / led_heartbeat      -> one LED behaviour each
 *   pwm_led_software / _hardware      -> one LED behaviour each
 *   board.h                           -> which pins the LEDs are wired to
 */

#include "blink_heartbeat_freertos_semaphores.h"

#include "blink_steady.h"
#include "board.h"
#include "led.h"
#include "led_heartbeat.h"
#include "log.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"

#include "FreeRTOS.h"
#include "semphr.h"
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
#define MONITOR_TOGGLE_MS       500u  /* blue LED blinks twice a second */
#define HEALTH_TIMEOUT_MS       100u  /* heartbeat must step at least this often */

/* Shared state: when each behaviour last ran --------------------------------*/
typedef struct {
    uint32_t heartbeatLastMs;
    uint32_t steadyLastMs;
    uint32_t pwmSwLastMs;
    uint32_t pwmHwLastMs;
} LedActivity_t;

/* Handles ------------------------------------------------------------------*/
static SemaphoreHandle_t s_activityMutex;   /* protects s_activity */
static SemaphoreHandle_t s_activityChanged; /* signals "new activity" */
static LedActivity_t     s_activity;
static LedHandle_t       s_monitorLed;

/* Behaviour tasks: step the LED, then record activity under the mutex -------*/
static void LedHeartbeat_TaskEntry(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_HEARTBEAT_MS));
        uint32_t nowMs = xTaskGetTickCount();
        LedHeartbeat_Task(nowMs);

        xSemaphoreTake(s_activityMutex, portMAX_DELAY);
        s_activity.heartbeatLastMs = nowMs;
        xSemaphoreGive(s_activityMutex);

        xSemaphoreGive(s_activityChanged);
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

        xSemaphoreTake(s_activityMutex, portMAX_DELAY);
        s_activity.steadyLastMs = nowMs;
        xSemaphoreGive(s_activityMutex);

        xSemaphoreGive(s_activityChanged);
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

        xSemaphoreTake(s_activityMutex, portMAX_DELAY);
        s_activity.pwmSwLastMs = nowMs;
        xSemaphoreGive(s_activityMutex);

        xSemaphoreGive(s_activityChanged);
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

        xSemaphoreTake(s_activityMutex, portMAX_DELAY);
        s_activity.pwmHwLastMs = nowMs;
        xSemaphoreGive(s_activityMutex);

        xSemaphoreGive(s_activityChanged);
    }
}

/* Monitor task: wakes on the semaphore, reads the shared struct, blinks ------*/
static void LedMonitor_TaskEntry(void *arg)
{
    (void)arg;
    uint32_t lastToggleMs = 0u;

    for (;;) {
        xSemaphoreTake(s_activityChanged, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();

        xSemaphoreTake(s_activityMutex, portMAX_DELAY);
        LedActivity_t snapshot = s_activity;
        xSemaphoreGive(s_activityMutex);

        /* Health check: the heartbeat must have stepped recently. */
        bool heartbeatAlive = (nowMs - snapshot.heartbeatLastMs) < HEALTH_TIMEOUT_MS;

        if (heartbeatAlive && (nowMs - lastToggleMs) >= MONITOR_TOGGLE_MS) {
            lastToggleMs = nowMs;
            Led_Toggle(&s_monitorLed);
        }
    }
}

/* ------------------------------------------------------------------------- */
void BlinkHeartbeatFreeRTOSSemaphores_Run(void)
{
    /* Start every behaviour before the scheduler runs. */
    if (!BlinkSteady_Init()) {
        log_error("SEMAPHORES: BlinkSteady_Init failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: BlinkSteady_Init OK");
    if (!LedHeartbeat_Init()) {
        log_error("SEMAPHORES: LedHeartbeat_Init failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: LedHeartbeat_Init OK");
    if (!PwmLedSoftware_Init()) {
        log_error("SEMAPHORES: PwmLedSoftware_Init failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: PwmLedSoftware_Init OK");
    if (!PwmLedHardware_Init()) {
        log_error("SEMAPHORES: PwmLedHardware_Init failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: PwmLedHardware_Init OK");

    /* Status LED for the monitor task (on-board blue LED, LD6). */
    const LedConfig_t monitorConfig = {
        .port = BOARD_LED_BLUE_PORT,
        .pin = BOARD_LED_BLUE_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };
    if (!Led_InitCustom(&s_monitorLed, &monitorConfig)) {
        log_error("SEMAPHORES: Led_InitCustom failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: monitor LED initialized");

    /* Synchronisation objects. */
    s_activityMutex = xSemaphoreCreateMutex();
    if (s_activityMutex == NULL) {
        log_error("SEMAPHORES: xSemaphoreCreateMutex failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: activity mutex created");
    s_activityChanged = xSemaphoreCreateBinary();
    if (s_activityChanged == NULL) {
        log_error("SEMAPHORES: xSemaphoreCreateBinary failed");
        Error_Handler();
    }
    log_debug("SEMAPHORES: activity-changed semaphore created");

    /* One task per behaviour. */
    if (xTaskCreate(LedHeartbeat_TaskEntry,   "led_hb",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_HEARTBEAT, NULL) != pdPASS) {
        log_error("SEMAPHORES: failed to create led_hb");
        Error_Handler();
    }
    log_debug("SEMAPHORES: created led_hb");
    if (xTaskCreate(BlinkSteady_TaskEntry,    "blink_st", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_STEADY,    NULL) != pdPASS) {
        log_error("SEMAPHORES: failed to create blink_st");
        Error_Handler();
    }
    log_debug("SEMAPHORES: created blink_st");
    if (xTaskCreate(PwmLedSoftware_TaskEntry, "pwm_sw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_SW,    NULL) != pdPASS) {
        log_error("SEMAPHORES: failed to create pwm_sw");
        Error_Handler();
    }
    log_debug("SEMAPHORES: created pwm_sw");
    if (xTaskCreate(PwmLedHardware_TaskEntry, "pwm_hw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_HW,    NULL) != pdPASS) {
        log_error("SEMAPHORES: failed to create pwm_hw");
        Error_Handler();
    }
    log_debug("SEMAPHORES: created pwm_hw");

    /* The monitor that reads the shared state. */
    if (xTaskCreate(LedMonitor_TaskEntry, "led_monitor", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_MONITOR, NULL) != pdPASS) {
        log_error("SEMAPHORES: failed to create led_monitor");
        Error_Handler();
    }
    log_debug("SEMAPHORES: created led_monitor");

    /* Hand control to the scheduler; only returns on a fatal error. */
    log_debug("SEMAPHORES: starting scheduler");
    vTaskStartScheduler();

    Error_Handler();
}
