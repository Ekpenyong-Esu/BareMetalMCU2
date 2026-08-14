/**
 * @file    blink_heartbeat_freertos_queues.c
 * @brief   Application 2 - Blink & Heartbeat on FreeRTOS, producer/consumer.
 *
 * The four LED behaviours are driven through ONE FreeRTOS queue:
 *   - 4 producer tasks, one per behaviour, each sends its behaviour id into
 *     the queue at the behaviour's cadence.
 *   - 1 consumer task blocks on the queue, reads the id, and runs that
 *     behaviour's step.
 *
 * Flow:  producer -> xQueueSend(id) -> queue -> xQueueReceive -> behaviour step
 * The queue decouples the producers from the consumer: producers never wait
 * for the LED work, and the consumer never decides when work happens.
 *
 * Layering:
 *   main.c                          -> chooses and runs the application
 *   blink_heartbeat_freertos_queues -> composition only (this module)
 *   blink_steady / led_heartbeat    -> one LED behaviour each
 *   pwm_led_software / _hardware    -> one LED behaviour each
 *   board.h                         -> which pins the LEDs are wired to
 */

#include "blink_heartbeat_freertos_queues.h"

#include "blink_steady.h"
#include "led_heartbeat.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Cadence (ms): how often each producer sends its id ------------------------*/
#define TASK_PERIOD_PWM_SW_MS     1u   /* finest the 1 kHz tick allows */
#define TASK_PERIOD_HEARTBEAT_MS  5u
#define TASK_PERIOD_PWM_HW_MS     5u
#define TASK_PERIOD_STEADY_MS    10u

/* Task priorities (higher number = higher priority) */
#define TASK_PRIO_PWM_SW          4    /* most timing-sensitive */
#define TASK_PRIO_HEARTBEAT       3
#define TASK_PRIO_PWM_HW          2
#define TASK_PRIO_STEADY          1
#define TASK_PRIO_CONSUMER        5    /* consumer serves every producer */

/* Task stack size in words (1 word = 4 bytes on Cortex-M4) */
#define TASK_STACK_SIZE_WORDS   256u

/* Queue depth: one slot per behaviour is enough (each sends at its own pace) */
#define EVENT_QUEUE_LENGTH         4u

/* Message carried by the queue: which behaviour to step ---------------------*/
typedef enum {
    BEHAVIOUR_HEARTBEAT,
    BEHAVIOUR_STEADY,
    BEHAVIOUR_PWM_SW,
    BEHAVIOUR_PWM_HW
} BehaviourId_t;

/* Handles ------------------------------------------------------------------*/
static QueueHandle_t s_eventQueue;

/* Producer tasks: one per behaviour -----------------------------------------*/
static void LedHeartbeat_Producer(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_HEARTBEAT_MS));
        BehaviourId_t message = BEHAVIOUR_HEARTBEAT;
        xQueueSend(s_eventQueue, &message, 0u);
    }
}

static void BlinkSteady_Producer(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_STEADY_MS));
        BehaviourId_t message = BEHAVIOUR_STEADY;
        xQueueSend(s_eventQueue, &message, 0u);
    }
}

static void PwmLedSoftware_Producer(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_PWM_SW_MS));
        BehaviourId_t message = BEHAVIOUR_PWM_SW;
        xQueueSend(s_eventQueue, &message, 0u);
    }
}

static void PwmLedHardware_Producer(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TASK_PERIOD_PWM_HW_MS));
        BehaviourId_t message = BEHAVIOUR_PWM_HW;
        xQueueSend(s_eventQueue, &message, 0u);
    }
}

/* Consumer task: reads the queue and runs the matching behaviour ------------*/
static void LedConsumer_TaskEntry(void *arg)
{
    (void)arg;
    BehaviourId_t message;

    for (;;) {
        xQueueReceive(s_eventQueue, &message, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();

        switch (message) {
            case BEHAVIOUR_HEARTBEAT:
                LedHeartbeat_Task(nowMs);
                break;
            case BEHAVIOUR_STEADY:
                BlinkSteady_Task(nowMs);
                break;
            case BEHAVIOUR_PWM_SW:
                PwmLedSoftware_Task(nowMs);
                break;
            case BEHAVIOUR_PWM_HW:
                PwmLedHardware_Task(nowMs);
                break;
        }
    }
}

/* ------------------------------------------------------------------------- */
void BlinkHeartbeatFreeRTOSQueues_Run(void)
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

    /* The single queue that connects the producers to the consumer. */
    s_eventQueue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(BehaviourId_t));
    if (s_eventQueue == NULL) {
        Error_Handler();
    }

    /* One producer task per behaviour. */
    if (xTaskCreate(LedHeartbeat_Producer,   "led_hb_p",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_HEARTBEAT, NULL) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(BlinkSteady_Producer,    "blink_st_p", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_STEADY,    NULL) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedSoftware_Producer, "pwm_sw_p",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_SW,    NULL) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedHardware_Producer, "pwm_hw_p",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_HW,    NULL) != pdPASS) {
        Error_Handler();
    }

    /* The single consumer that does all the LED work. */
    if (xTaskCreate(LedConsumer_TaskEntry, "led_consumer", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_CONSUMER, NULL) != pdPASS) {
        Error_Handler();
    }

    /* Hand control to the scheduler; only returns on a fatal error. */
    vTaskStartScheduler();

    Error_Handler();
}