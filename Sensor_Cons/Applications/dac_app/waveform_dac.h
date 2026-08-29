/**
 * @file waveform_dac.h
 * @brief DAC-side setup for the waveform app (init + DMA streaming)
 */

#ifndef WAVEFORM_DAC_H
#define WAVEFORM_DAC_H

#include "dac.h"
#include "waveform_lut.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the DAC for TIM7-TRGO-paced output. */
bool Waveform_DacInit(DAC_HandleStruct *dac);

/** Stream one WAVEFORM_LUT_SIZE table to the DAC, repeating until replaced. */
bool Waveform_DacStream(DAC_HandleStruct *dac, uint32_t *samples);

/** Channel used by the waveform app. */
#define WAVEFORM_DAC_CHANNEL  DAC_CHANNEL_1

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_DAC_H */
