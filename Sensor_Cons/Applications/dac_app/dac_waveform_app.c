/**
 * @file dac_waveform_app.c
 * @brief The DAC application: waveform generator example, TIM6-TRGO driven.
 *
 * Orchestrates independent modules:
 *   waveform_lut     builds the sample tables (pure math),
 *   waveform_dac     owns the DAC config/arming,
 *   waveform_timer   owns the TIM6 pacing counter and TRGO routing,
 *   waveform_display owns the USART1 output.
 *
 * Cycles sine -> triangle -> sawtooth on DAC_OUT1 (PA4), switching every
 * CYCLES_PER_WAVEFORM periods.
 *
 * TIM6 fires TRGO on its update event; each trigger clocks the next LUT
 * sample into the DAC, so timing is hardware-exact. With PSC=83 (84 MHz /
 * 1 MHz) and ARR=2000 the update period is 2 ms, so 100 samples round out
 * every 200 ms -> 5 Hz, same cadence as the HAL_Delay variant but without
 * busy-waiting.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   dac_waveform_app    -> orchestration only (this module)
 *   waveform_lut, waveform_dac, waveform_timer, waveform_display, dac, tim
 *                       -> independent modules
 */

#include "dac_waveform_app.h"

#include "tim.h"
#include "waveform_dac.h"
#include "waveform_display.h"
#include "waveform_lut.h"
#include "waveform_timer.h"

#include "dac.h"

#include <stdint.h>

/* Print every Nth sample: a blocking 115200 line takes ~2.5 ms, longer than
 * the 2 ms tick, so printing every sample would stall the pump. */
#define PRINT_EVERY_N_SAMPLES  20U

/* How long each waveform runs before switching (200 ms per cycle). */
#define CYCLES_PER_WAVEFORM    10U

/* Handles --------------------------------------------------------------- */
static DAC_HandleStruct s_dac;
static TIM_HandleTypeDef s_tim;
static Waveform_Display_t s_display;

/* All tables are built up front so switching never stalls the sample pump. */
static uint32_t s_lut[WAVEFORM_TYPE_COUNT][WAVEFORM_LUT_SIZE];

void DacWaveformApp_Run(void)
{
    if (!Waveform_DisplayInit(&s_display)) {
        return;
    }
    if (!Waveform_DacInit(&s_dac)) {
        return;
    }
    if (!Waveform_TimerInit(&s_tim)) {
        return;
    }
    if (!Waveform_TimerRouteTrigger(&s_tim)) {
        return;
    }

    for (uint32_t t = 0; t < WAVEFORM_TYPE_COUNT; t++) {
        WaveformLut_Build(s_lut[t], (Waveform_Type_t)t);
    }

    Waveform_Type_t type = WAVEFORM_SINE;

    if (!Waveform_DacArmStart(&s_dac, s_lut[type][0])) {
        return;
    }
    if (TIM_Start(&s_tim) != HAL_OK) {
        return;
    }

    Waveform_DisplayBanner(&s_display, WaveformLut_Name(type));

    uint32_t index = 1U;
    uint32_t cycles = 0U;

    for (;;) {
        if (Waveform_TimerTickArrived(&s_tim)) {
            Waveform_TimerAckTick(&s_tim);
            DAC_SetValue(&s_dac, WAVEFORM_DAC_CHANNEL, s_lut[type][index]);

            if ((index % PRINT_EVERY_N_SAMPLES) == 0U) {
                Waveform_DisplayShow(&s_display, index, s_lut[type][index]);
            }

            index = (index + 1U) % WAVEFORM_LUT_SIZE;

            if (index == 0U && ++cycles == CYCLES_PER_WAVEFORM) {
                cycles = 0U;
                type = (Waveform_Type_t)((type + 1U) % WAVEFORM_TYPE_COUNT);
                Waveform_DisplayBanner(&s_display, WaveformLut_Name(type));
            }
        }
    }
}
