/**
 * @file waveform_dac.c
 * @brief DAC-side setup for the waveform app
 */

#include "waveform_dac.h"

#include "log.h"

bool Waveform_DacInit(DAC_HandleStruct *dac)
{
    const DAC_ConfigTypeDef dacConfig = {
        .channel       = WAVEFORM_DAC_CHANNEL,
        .trigger       = DAC_TRIGGER_T7_TRGO,
        .output_buffer = DAC_OUTPUTBUFFER_ENABLE,
    };

    if (DAC_Init(dac, &dacConfig) != HAL_OK) {
        log_error("DAC waveform: DAC init failed");
        return false;
    }
    return true;
}

bool Waveform_DacStream(DAC_HandleStruct *dac, uint32_t *samples)
{
    /* Ignored on the first call, when nothing is streaming yet. */
    (void)DAC_StopDMA(dac, WAVEFORM_DAC_CHANNEL);

    if (DAC_StartCircularDMA(dac, WAVEFORM_DAC_CHANNEL, samples,
                             WAVEFORM_LUT_SIZE) != HAL_OK) {
        log_error("DAC waveform: DMA stream start failed");
        return false;
    }
    return true;
}
