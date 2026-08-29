/**
 * @file dac_waveform_app.c
 * @brief The DAC application: waveform generator example, DMA-streamed.
 *
 * Nothing in software sits on the sample path. TIM7 raises TRGO every 2 ms,
 * each trigger makes the DAC pull one word from the LUT over DMA, and the
 * circular buffer repeats forever. The CPU is not involved per sample, so the
 * output cannot jitter or slip no matter what the scheduler is doing.
 *
 * That leaves the tasks doing only policy:
 *   - waveform task: every WAVEFORM_HOLD_MS, points the DMA at the next table
 *     and announces the change on the queue.
 *   - display task: reports the level actually present on the pin, and prints
 *     a banner whenever the queue tells it the shape changed.
 *
 * Flow:  TIM7 TRGO -> DMA -> DAC -> PA4        (hardware only)
 *        waveform task -> queue -> display task (software, off the sample path)
 *
 * The display reads DAC_GetValue rather than a software index, so what it
 * prints is what the pin is really doing.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   dac_waveform_app    -> composition only (this module)
 *   waveform_lut        -> sample tables (pure math)
 *   waveform_dac        -> DAC config + DMA streaming
 *   waveform_timer      -> TIM7 trigger source
 *   waveform_display    -> UART setup + formatted output
 */

#include "dac_waveform_app.h"

#include "main.h"
#include "dac.h"
#include "waveform_dac.h"
#include "waveform_display.h"
#include "waveform_lut.h"
#include "waveform_timer.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <stdint.h>

/* How long each waveform runs before switching (200 ms per cycle). */
#define WAVEFORM_HOLD_MS      2000U

/* How often the console reports the output level. */
#define DISPLAY_PERIOD_MS     250U

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_WAVEFORM    2
#define TASK_PRIO_DISPLAY     1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS 256u

#define WAVEFORM_QUEUE_LENGTH 4u

/* Handles --------------------------------------------------------------- */
static QueueHandle_t      s_waveformQueue;
static DAC_HandleStruct   s_dac;
static Waveform_Display_t s_display;

/* Handed to the DMA, so these must live as long as the transfer does. */
static uint32_t s_lut[WAVEFORM_TYPE_COUNT][WAVEFORM_LUT_SIZE];

/* Policy: rotate the shape the DMA is streaming --------------------------- */
static void WaveformTask(void *arg)
{
    (void)arg;

    TickType_t      lastWake = xTaskGetTickCount();
    Waveform_Type_t type     = WAVEFORM_SINE;

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(WAVEFORM_HOLD_MS));

        type = (Waveform_Type_t)((type + 1U) % WAVEFORM_TYPE_COUNT);

        if (!Waveform_DacStream(&s_dac, s_lut[type])) {
            Error_Handler();
        }

        xQueueSend(s_waveformQueue, &type, 0u);
    }
}

/* Reports the level actually on the pin ----------------------------------- */
static void DisplayTask(void *arg)
{
    (void)arg;

    Waveform_Type_t type;

    for (;;) {
        /* Doubles as the print interval: a shape change cuts the wait short. */
        if (xQueueReceive(s_waveformQueue, &type,
                          pdMS_TO_TICKS(DISPLAY_PERIOD_MS)) == pdTRUE) {
            Waveform_DisplayBanner(&s_display, WaveformLut_Name(type));
        }

        Waveform_DisplayLevel(&s_display, DAC_GetValue(&s_dac, WAVEFORM_DAC_CHANNEL));
    }
}

/* ------------------------------------------------------------------------ */
void DacWaveformApp_Run(void)
{
    if (!Waveform_DisplayInit(&s_display)) {
        Error_Handler(); /* no way to report anything; nothing else to do */
    }

    if (!Waveform_DacInit(&s_dac)) {
        Error_Handler(); /* nothing to drive PA4 with */
    }

    for (uint32_t t = 0; t < WAVEFORM_TYPE_COUNT; t++) {
        WaveformLut_Build(s_lut[t], (Waveform_Type_t)t);
    }

    if (!Waveform_DacStream(&s_dac, s_lut[WAVEFORM_SINE])) {
        Error_Handler();
    }

    /* Started last: the DMA must be armed before the first trigger arrives. */
    if (!Waveform_TimerStart()) {
        Error_Handler();
    }

    Waveform_DisplayBanner(&s_display, WaveformLut_Name(WAVEFORM_SINE));

    s_waveformQueue = xQueueCreate(WAVEFORM_QUEUE_LENGTH, sizeof(Waveform_Type_t));
    if (s_waveformQueue == NULL) {
        Error_Handler();
    }

    if (xTaskCreate(WaveformTask, "waveform", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_WAVEFORM, NULL) != pdPASS) {
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
