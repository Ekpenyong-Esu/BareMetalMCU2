/**
 * @file dac_waveform_app.h
 * @brief The DAC application: waveform generator example, TIM7-TRGO driven
 */

#ifndef DAC_WAVEFORM_APP_H
#define DAC_WAVEFORM_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Cycle sine, triangle and sawtooth on DAC_OUT1 (PA4), TIM7-TRGO paced.
 *  Starts the FreeRTOS scheduler and does not return. */
void DacWaveformApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* DAC_WAVEFORM_APP_H */
