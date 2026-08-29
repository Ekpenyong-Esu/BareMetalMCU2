/**
 * @file dac_waveform_app.c
 * @brief The DAC application: waveform generator example, running on FreeRTOS.
 *
 * Two tasks and one queue:
 *   - waveform task (producer): woken by the TIM7 update ISR, writes the next
 *     LUT sample to the DAC, and posts a copy to the queue.
 *   - display task (consumer): blocks on the queue and prints over USART1.
 *
 * Flow:  TIM7 IRQ -> notify -> DAC write -> queue -> UART
 *
 * The queue keeps the producer off the UART: a blocking 115200 line (~2.5 ms)
 * outlasts the 2 ms sample period, so printing inline would disturb the
 * waveform.
 *
 * Taking the tick from the timer's own interrupt rather than from
 * vTaskDelayUntil means the sample rate is set by TIM7 alone, so it is neither
 * quantised to the 1 kHz RTOS tick nor able to slip a sample when the
 * scheduler runs late.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   dac_waveform_app    -> composition only (this module)
 *   waveform_lut        -> sample tables (pure math)
 *   waveform_dac        -> DAC config + arming
 *   waveform_timer      -> TIM7 trigger source
 *   waveform_display    -> UART setup + formatted sample output
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
#define CYCLES_PER_WAVEFORM   10U

/* Post every Nth sample to the console; 500 lines/s is unreadable. */
#define PRINT_EVERY_N_SAMPLES 20U

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_WAVEFORM    2
#define TASK_PRIO_DISPLAY     1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS 256u

/* Shallow on purpose: the UART cannot keep up with the sample rate, so a
 * longer queue would only buffer stale samples. */
#define SAMPLE_QUEUE_LENGTH   8u

typedef struct {
    uint32_t        index;
    uint32_t        code;
    Waveform_Type_t type;
} WaveformSample_t;

/* Handles --------------------------------------------------------------- */
static QueueHandle_t      s_sampleQueue;
static DAC_HandleStruct   s_dac;
static Waveform_Display_t s_display;

/* All tables are built up front so switching never stalls the producer. */
static uint32_t s_lut[WAVEFORM_TYPE_COUNT][WAVEFORM_LUT_SIZE];

static TaskHandle_t s_waveformTask;

/* ISR: hand the tick to the waveform task and get out -------------------- */
static void OnTimerTick(void)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(s_waveformTask, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

/* Producer: one DAC sample per TIM7 update ------------------------------- */
static void WaveformTask(void *arg)
{
    (void)arg;

    Waveform_Type_t type   = WAVEFORM_SINE;
    uint32_t        index  = 0U;
    uint32_t        cycles = 0U;

    if (!Waveform_TimerStart(OnTimerTick)) {
        Error_Handler();
    }

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        const uint32_t code = s_lut[type][index];
        DAC_SetValue(&s_dac, WAVEFORM_DAC_CHANNEL, code);

        if ((index % PRINT_EVERY_N_SAMPLES) == 0U) {
            const WaveformSample_t sample = {index, code, type};
            xQueueSend(s_sampleQueue, &sample, 0u); /* dropped if full: by design */
        }

        index = (index + 1U) % WAVEFORM_LUT_SIZE;

        /* Switch shapes only at a cycle boundary, so the output never takes a
         * step discontinuity mid-period. */
        if (index == 0U && ++cycles == CYCLES_PER_WAVEFORM) {
            cycles = 0U;
            type   = (Waveform_Type_t)((type + 1U) % WAVEFORM_TYPE_COUNT);
        }
    }
}

/* Consumer: waits for a sample and prints it over UART ------------------- */
static void DisplayTask(void *arg)
{
    (void)arg;

    WaveformSample_t sample;
    Waveform_Type_t  shown = WAVEFORM_TYPE_COUNT; /* forces a banner first time */

    for (;;) {
        if (xQueueReceive(s_sampleQueue, &sample, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        if (sample.type != shown) {
            shown = sample.type;
            Waveform_DisplayBanner(&s_display, WaveformLut_Name(shown));
        }

        Waveform_DisplayShow(&s_display, sample.index, sample.code);
    }
}

/* ------------------------------------------------------------------------ */
void DacWaveformApp_Run(void)
{
    if (!Waveform_DisplayInit(&s_display)) {
        Error_Handler(); /* no way to report samples; nothing else to do */
    }

    if (!Waveform_DacInit(&s_dac)) {
        Error_Handler(); /* nothing to drive PA4 with */
    }

    for (uint32_t t = 0; t < WAVEFORM_TYPE_COUNT; t++) {
        WaveformLut_Build(s_lut[t], (Waveform_Type_t)t);
    }

    if (!Waveform_DacArmStart(&s_dac, s_lut[WAVEFORM_SINE][0])) {
        Error_Handler();
    }

    s_sampleQueue = xQueueCreate(SAMPLE_QUEUE_LENGTH, sizeof(WaveformSample_t));
    if (s_sampleQueue == NULL) {
        Error_Handler();
    }

    /* The task starts the timer itself, so it exists before the first IRQ. */
    if (xTaskCreate(WaveformTask, "waveform", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_WAVEFORM, &s_waveformTask) != pdPASS) {
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
