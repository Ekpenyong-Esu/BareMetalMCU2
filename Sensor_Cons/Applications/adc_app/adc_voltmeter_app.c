/**
 * @file adc_voltmeter_app.c
 * @brief The ADC application: voltmeter example, running on FreeRTOS.
 *
 * Producer/consumer via ONE FreeRTOS queue:
 *   - reader task (producer): samples PA0 (ADC1 channel 0) once a second and
 *     sends the voltage into the queue.
 *   - display task (consumer): blocks on the queue and prints each sample
 *     over USART1 (115200 8N1).
 *
 * Flow:  ADC -> xQueueSend(voltage) -> queue -> xQueueReceive -> UART
 * The queue decouples the two tasks: the reader never waits on UART, and the
 * display task never decides when the next sample is taken.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   adc_voltmeter_app   -> composition only (this module)
 *   voltage_reader       -> ADC setup + single-channel voltage reads
 *   voltage_display      -> UART setup + formatted voltage output
 */

#include "adc_voltmeter_app.h"

#include "main.h"
#include "voltage_display.h"
#include "voltage_reader.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Sample cadence (ms): how often the reader task takes a reading. */
#define READER_PERIOD_MS      1000u

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_READER      2
#define TASK_PRIO_DISPLAY     1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS 256u

/* Queue depth: the display task drains readings faster than they arrive. */
#define VOLTAGE_QUEUE_LENGTH  4u

/* Handles --------------------------------------------------------------- */
static QueueHandle_t   s_voltageQueue;
static VoltageReader_t s_reader;
static VoltageDisplay_t s_display;

/* Producer: samples the ADC and sends each reading into the queue -------- */
static void ReaderTask(void *arg)
{
    (void)arg;

    for (;;) {
        float voltage = VoltageReader_Read(&s_reader);
        xQueueSend(s_voltageQueue, &voltage, 0u);
        vTaskDelay(pdMS_TO_TICKS(READER_PERIOD_MS));
    }
}

/* Consumer: waits for a reading and prints it over UART ------------------ */
static void DisplayTask(void *arg)
{
    (void)arg;
    float voltage = 0.0f;

    for (;;) {
        if (xQueueReceive(s_voltageQueue, &voltage, portMAX_DELAY) == pdTRUE) {
            VoltageDisplay_Show(&s_display, voltage);
        }
    }
}

/* ------------------------------------------------------------------------ */
void AdcVoltmeterApp_Run(void)
{
    if (!VoltageDisplay_Init(&s_display)) {
        Error_Handler(); /* no way to report readings; nothing else to do */
    }

    if (!VoltageReader_Init(&s_reader)) {
        Error_Handler(); /* nothing to measure with */
    }

    s_voltageQueue = xQueueCreate(VOLTAGE_QUEUE_LENGTH, sizeof(float));
    if (s_voltageQueue == NULL) {
        Error_Handler();
    }

    if (xTaskCreate(ReaderTask, "reader", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_READER, NULL) != pdPASS) {
        Error_Handler();
    }

    if (xTaskCreate(DisplayTask, "display", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_DISPLAY, NULL) != pdPASS) {
        Error_Handler();
    }

    /* Hand control to the scheduler; only returns on a fatal error. */
    vTaskStartScheduler();

    Error_Handler();
}
