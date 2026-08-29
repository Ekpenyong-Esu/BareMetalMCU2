/**
 * @file dac_waveform_app.h
 * @brief The DAC application: waveform generator example, TIM6-TRGO driven
 */

#ifndef DAC_WAVEFORM_APP_H
#define DAC_WAVEFORM_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Cycle sine, triangle and sawtooth on DAC_OUT1 (PA4), TIM6-TRGO paced. */
void DacWaveformApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* DAC_WAVEFORM_APP_H */
