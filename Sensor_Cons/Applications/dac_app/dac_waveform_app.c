/**
 * @file dac_waveform_app.c
 * @brief The DAC application: waveform generator example, running on FreeRTOS.
 *
 * ISR offloading + producer/consumer via ONE FreeRTOS queue:
 *   - TIM7 update ISR: notifies the pump task, nothing else.
 *   - pump task (producer): wakes on the notification, writes the next LUT
 *     sample to the DAC, and offers the sample to the queue.
 *   - display task (consumer): blocks on the queue and prints over USART1.
 *
 * Flow:  TIM7 IRQ -> notify -> DAC write -> queue -> UART
 *
 * The queue is what makes this version better than the super-loop one. There a
 * blocking 115200 line (~2.5 ms) outlasted the 2 ms tick, so printing had to be
 * decimated by hand to keep the pump fed. Here the pump never touches the UART:
 * it offers samples with zero timeout and drops them when the queue is full, so
 * the waveform stays exact and the console self-throttles.
 *
 * Waveform rotation stays in the pump task, at a cycle boundary, so switching
 * shapes cannot put a step discontinuity in the middle of a period.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   dac_waveform_app    -> composition only (this module)
 *   waveform_lut        -> sample tables (pure math)
 *   waveform_dac        -> DAC config + arming
 *   waveform_timer      -> TIM7 pacing, TRGO routing, update ISR
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

/* Offer every Nth sample to the console. Unlike the super-loop branch this is
 * purely for readability -- 500 lines/s is unreadable -- not to protect the
 * sample timing, which the queue already does. */
#define PRINT_EVERY_N_SAMPLES 20U

/* Task priorities (higher number = higher priority). */
#define TASK_PRIO_PUMP        3
#define TASK_PRIO_DISPLAY     1

/* Task stack size in words (1 word = 4 bytes on Cortex-M4). */
#define TASK_STACK_SIZE_WORDS 256u

/* Shallow on purpose: the UART cannot keep up with a 500 Hz sample rate, so a
 * long queue would only buffer stale samples. */
#define SAMPLE_QUEUE_LENGTH   8u

typedef struct {
    uint32_t        index;
    uint32_t        code;
    Waveform_Type_t type;
} WaveformSample_t;

/* Handles --------------------------------------------------------------- */
static QueueHandle_t      s_sampleQueue;
static TaskHandle_t       s_pumpTask;
static DAC_HandleStruct   s_dac;
static Waveform_Display_t s_display;

/* All tables are built up front so switching never stalls the sample pump. */
static uint32_t s_lut[WAVEFORM_TYPE_COUNT][WAVEFORM_LUT_SIZE];

/* ISR: hand the tick to the pump task and get out ------------------------ */
static void OnTimerTick(void)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(s_pumpTask, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

/* Producer: one DAC sample per tick, never blocks ------------------------ */
static void PumpTask(void *arg)
{
    (void)arg;

    Waveform_Type_t type   = WAVEFORM_SINE;
    uint32_t        index  = 1U;
    uint32_t        cycles = 0U;

    if (!Waveform_DacArmStart(&s_dac, s_lut[type][0])) {
        Error_Handler();
    }

    Waveform_TimerOnTick(OnTimerTick);

    if (!Waveform_TimerStart()) {
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
    Waveform_Type_t  shown = WAVEFORM_TYPE_COUNT; /* forces a first banner */

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

    if (!Waveform_TimerInit()) {
        Error_Handler();
    }

    for (uint32_t t = 0; t < WAVEFORM_TYPE_COUNT; t++) {
        WaveformLut_Build(s_lut[t], (Waveform_Type_t)t);
    }

    s_sampleQueue = xQueueCreate(SAMPLE_QUEUE_LENGTH, sizeof(WaveformSample_t));
    if (s_sampleQueue == NULL) {
        Error_Handler();
    }

    /* The pump task starts the timer, so it must exist before the first IRQ. */
    if (xTaskCreate(PumpTask, "pump", TASK_STACK_SIZE_WORDS, NULL,
                    TASK_PRIO_PUMP, &s_pumpTask) != pdPASS) {
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
