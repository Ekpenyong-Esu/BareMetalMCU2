/**
 * @file dac_waveform_app.c
 * @brief The DAC application: waveform generator example
 *
 * Outputs a sine wave on DAC_OUT1 (PA4). Wire a scope or the DAC pin to an
 * LED (through a resistor) to see/hear the result.
 *
 * This is the "ordinary" version: no timer trigger, no DMA. A lookup table
 * is precomputed once, then the main loop writes one sample at a time and
 * paces itself with HAL_Delay. That is plenty for a slow, visible waveform;
 * a FreeRTOS or timer/DMA-driven version can push the sample rate much higher
 * without keeping the CPU busy in a delay loop.
 */

#include "dac_waveform_app.h"

#include "dac.h"
#include <math.h>
#include <stdint.h>

#define DAC_WAVEFORM_APP_CHANNEL       DAC_CHANNEL_1
#define DAC_WAVEFORM_SAMPLE_COUNT      100U  /* points per sine cycle */
#define DAC_WAVEFORM_SAMPLE_PERIOD_MS  2U    /* 100 * 2 ms = 200 ms period -> 5 Hz */

static DAC_HandleStruct s_dac;
static uint32_t s_sineTable[DAC_WAVEFORM_SAMPLE_COUNT];

/* One-time lookup table: sample i holds the DAC code for angle 2*pi*i/N. */
static void BuildSineTable(void)
{
    for (uint32_t i = 0; i < DAC_WAVEFORM_SAMPLE_COUNT; i++) {
        float angle = (2.0f * (float)M_PI * (float)i) / (float)DAC_WAVEFORM_SAMPLE_COUNT;
        float unit  = (sinf(angle) + 1.0f) * 0.5f; /* rescale -1..1 to 0..1 */
        s_sineTable[i] = (uint32_t)(unit * (float)DAC_MAX_VALUE_12BIT);
    }
}

void DacWaveformApp_Run(void)
{
    const DAC_ConfigTypeDef dacConfig = {
        .channel       = DAC_WAVEFORM_APP_CHANNEL,
        .trigger       = DAC_TRIGGER_NONE,
        .output_buffer = DAC_OUTPUTBUFFER_ENABLE,
    };

    if (DAC_Init(&s_dac, &dacConfig) != HAL_OK) {
        return; /* nothing to output to */
    }

    BuildSineTable();

    for (;;) {
        for (uint32_t i = 0; i < DAC_WAVEFORM_SAMPLE_COUNT; i++) {
            DAC_SetValue(&s_dac, DAC_WAVEFORM_APP_CHANNEL, s_sineTable[i]);
            HAL_Delay(DAC_WAVEFORM_SAMPLE_PERIOD_MS);
        }
    }
}
