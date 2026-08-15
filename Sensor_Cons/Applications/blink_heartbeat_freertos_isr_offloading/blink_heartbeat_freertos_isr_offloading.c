/**
 * @file    blink_heartbeat_freertos_isr_offloading.c
 * @brief   Application 5 - Blink & Heartbeat on FreeRTOS, ISR offloading.
 *
 * Demonstrates ISR offloading (deferred interrupt handling): an interrupt
 * handler must run as fast as possible, so it only signals a task and lets
 * the task do the real work.
 *
 * Event source: a hardware timer (TIM7) interrupts at 1 kHz - the fastest
 * behaviour cadence. TIM7_IRQHandler is the ISR:
 *
 *   - It clears the timer flag and counts milliseconds.
 *   - At each behaviour's cadence it wakes that behaviour's task with
 *         xTaskNotifyFromISR(s_task, 0, eNoAction, &woken)
 *   - It never touches an LED. All LED work is deferred to the tasks.
 *
 *   - 4 behaviour tasks, one per LED behaviour. Each blocks on
 *         xTaskNotifyWait(0, 0, NULL, portMAX_DELAY)
 *     and steps its LED when the ISR wakes it.
 *
 * Flow per cadence:  TIM7 fires -> ISR -> xTaskNotifyFromISR
 *                    -> behaviour task wakes -> steps the LED
 *
 * Layering:
 *   main.c                                          -> chooses the application
 *   blink_heartbeat_freertos_isr_offloading         -> composition + the ISR
 *   blink_steady / led_heartbeat                    -> one LED behaviour each
 *   pwm_led_software / _hardware                    -> one LED behaviour each
 *   board.h                                         -> which pins the LEDs are on
 */

#include "blink_heartbeat_freertos_isr_offloading.h"

#include "blink_steady.h"
#include "led_heartbeat.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"
#include "tim_base.h"
#include "tim_clock.h"

#include "FreeRTOS.h"
#include "task.h"

#include "stm32f4xx_hal.h"

/* Cadence (ms): the ISR wakes each behaviour task this often -----------------*/
#define CADENCE_PWM_SW_MS      1u   /* every TIM7 tick (the finest cadence) */
#define CADENCE_HEARTBEAT_MS   5u
#define CADENCE_PWM_HW_MS      5u
#define CADENCE_STEADY_MS     10u

/* Task priorities (higher number = higher priority) */
#define TASK_PRIO_PWM_SW          4    /* most timing-sensitive */
#define TASK_PRIO_HEARTBEAT       3
#define TASK_PRIO_PWM_HW          2
#define TASK_PRIO_STEADY          1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4) */
#define TASK_STACK_SIZE_WORDS   256u

/* TIM7: 1 kHz interrupt source ----------------------------------------------*/
/* ISR priority must be at or below configMAX_SYSCALL_INTERRUPT_PRIORITY (5)
 * so xTaskNotifyFromISR() is allowed. 6 is the lowest "safe" priority. */
#define TIM7_IRQ_PRIORITY        6u
#define TIM7_PRESCALER          84u - 1u   /* 84 MHz / 84 = 1 MHz counter */
#define TIM7_PERIOD            1000u - 1u  /* 1 MHz / 1000 = 1 kHz (1 ms) */

/* Handles ------------------------------------------------------------------*/
static TIM_HandleTypeDef s_timer7;        /* the 1 kHz hardware event source */
static TaskHandle_t s_heartbeatTask;      /* woken by the ISR at its cadence */
static TaskHandle_t s_steadyTask;
static TaskHandle_t s_pwmSwTask;
static TaskHandle_t s_pwmHwTask;
static volatile uint32_t s_isrMs;         /* milliseconds counted in the ISR */

/* The ISR: deferred interrupt handling --------------------------------------*/

static bool Timer7_Start(void)
{
    /* TIM7 is a basic timer on APB1. APB1 = 42 MHz, so the timer clock is
     * doubled to 84 MHz (same math as HAL_InitTick uses for TIM6). */
    if (!TIM_Clock_Enable(TIM7)) {
        return false;
    }

    if (TIM_Init(&s_timer7, TIM7, TIM7_PRESCALER, TIM7_PERIOD) != HAL_OK) {
        return false;
    }

    /* ISR priority must be at or below configMAX_SYSCALL_INTERRUPT_PRIORITY
     * (5) so xTaskNotifyFromISR() is allowed. 6 is the lowest "safe" priority.
     * Note: HAL_TIM_Base_Start_IT() (via TIM_Start_IT) enables the timer's
     * update interrupt but NOT the NVIC line, so we enable it here. */
    HAL_NVIC_SetPriority(TIM7_IRQn, TIM7_IRQ_PRIORITY, 0u);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);

    return (TIM_Start_IT(&s_timer7) == HAL_OK);
}

/**
 * @brief  TIM7 update interrupt. This is the ISR, so it must stay short.
 * @note   Fires 1000x/second. It only counts time and wakes behaviour tasks;
 *         the actual LED work happens in the tasks, not here.
 */
void TIM7_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&s_timer7, TIM_FLAG_UPDATE) == RESET) {
        return;
    }
    __HAL_TIM_CLEAR_FLAG(&s_timer7, TIM_FLAG_UPDATE);

    s_isrMs++;
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    /* Wake the 1 ms behaviour every tick, the others at their cadence. */
    xTaskNotifyFromISR(s_pwmSwTask, 0u, eNoAction, &higherPriorityTaskWoken);

    if ((s_isrMs % CADENCE_HEARTBEAT_MS) == 0u) {
        xTaskNotifyFromISR(s_heartbeatTask, 0u, eNoAction, &higherPriorityTaskWoken);
    }
    if ((s_isrMs % CADENCE_PWM_HW_MS) == 0u) {
        xTaskNotifyFromISR(s_pwmHwTask, 0u, eNoAction, &higherPriorityTaskWoken);
    }
    if ((s_isrMs % CADENCE_STEADY_MS) == 0u) {
        xTaskNotifyFromISR(s_steadyTask, 0u, eNoAction, &higherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

/* Behaviour tasks: block until the ISR wakes them, then step the LED --------*/
static void LedHeartbeat_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xTaskNotifyWait(0u, 0u, NULL, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        LedHeartbeat_Task(nowMs);
    }
}

static void BlinkSteady_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xTaskNotifyWait(0u, 0u, NULL, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        BlinkSteady_Task(nowMs);
    }
}

static void PwmLedSoftware_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xTaskNotifyWait(0u, 0u, NULL, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedSoftware_Task(nowMs);
    }
}

static void PwmLedHardware_TaskEntry(void *arg)
{
    (void)arg;

    for (;;) {
        xTaskNotifyWait(0u, 0u, NULL, portMAX_DELAY);
        uint32_t nowMs = xTaskGetTickCount();
        PwmLedHardware_Task(nowMs);
    }
}

/* ------------------------------------------------------------------------- */


void BlinkHeartbeatFreeRTOSIsrOffloading_Run(void)
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

    /* One task per behaviour; the ISR wakes each at its cadence. */
    if (xTaskCreate(LedHeartbeat_TaskEntry,   "led_hb",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_HEARTBEAT, &s_heartbeatTask) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(BlinkSteady_TaskEntry,    "blink_st", TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_STEADY,    &s_steadyTask) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedSoftware_TaskEntry, "pwm_sw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_SW,    &s_pwmSwTask) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(PwmLedHardware_TaskEntry, "pwm_hw",   TASK_STACK_SIZE_WORDS, NULL, TASK_PRIO_PWM_HW,    &s_pwmHwTask) != pdPASS) {
        Error_Handler();
    }

    /* Start the 1 kHz hardware timer that drives everything. */
    if (!Timer7_Start()) {
        Error_Handler();
    }

    /* Hand control to the scheduler; only returns on a fatal error. */
    vTaskStartScheduler();

    Error_Handler();
}
