/**
 * @file waveform_dac.h
 * @brief DAC-side setup for the waveform app (init + arming)
 */

#ifndef WAVEFORM_DAC_H
#define WAVEFORM_DAC_H

#include "dac.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the DAC for TIM6-TRGO-paced output. */
bool Waveform_DacInit(DAC_HandleStruct *dac);

/** Arm the DAC with its first sample and start it. */
bool Waveform_DacArmStart(DAC_HandleStruct *dac, uint32_t firstCode);

/** Channel used by the waveform app. */
#define WAVEFORM_DAC_CHANNEL  DAC_CHANNEL_1

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_DAC_H */
