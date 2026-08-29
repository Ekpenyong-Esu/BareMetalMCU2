/**
 * @file dac_waveform_app.c
 * @brief The DAC application: waveform generator example, TIM6-TRGO driven.
 *
 * Orchestrates independent modules:
 *   sine_lut      builds the waveform sample table (pure math),
 *   waveform_dac  owns the DAC config/arming,
 *   waveform_timer owns the TIM6 pacing counter and TRGO routing.
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
 *   sine_lut, waveform_dac, waveform_timer, dac, tim -> independent modules
 */

#include "dac_waveform_app.h"

#include "sine_lut.h"
#include "tim.h"
#include "waveform_dac.h"
#include "waveform_display.h"
#include "waveform_timer.h"

#include "dac.h"

#include <stdint.h>

/* Print every Nth sample: a blocking 115200 line takes ~2.5 ms, longer than
 * the 2 ms tick, so printing every sample would stall the pump. */
#define PRINT_EVERY_N_SAMPLES  20U

/* Handles --------------------------------------------------------------- */
static DAC_HandleStruct s_dac;
static TIM_HandleTypeDef s_tim;
static Waveform_Display_t s_display;
static uint32_t s_lut[SINE_LUT_SIZE];

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

    SineLut_Build(s_lut);

    if (!Waveform_DacArmStart(&s_dac, s_lut[0])) {
        return;
    }
    if (TIM_Start(&s_tim) != HAL_OK) {
        return;
    }

    uint32_t index = 1U;

    for (;;) {
        if (Waveform_TimerTickArrived(&s_tim)) {
            Waveform_TimerAckTick(&s_tim);
            DAC_SetValue(&s_dac, WAVEFORM_DAC_CHANNEL, s_lut[index]);

            if ((index % PRINT_EVERY_N_SAMPLES) == 0U) {
                Waveform_DisplayShow(&s_display, index, s_lut[index]);
            }

            index = (index + 1U) % SINE_LUT_SIZE;
        }
    }
}
