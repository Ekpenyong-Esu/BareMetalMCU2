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

bool Waveform_DacArmStart(DAC_HandleStruct *dac, uint32_t firstCode)
{
    if (HAL_DAC_SetValue(&dac->hal_handle, WAVEFORM_DAC_CHANNEL,
                         DAC_ALIGN_12B_R, firstCode) != HAL_OK) {
        return false;
    }
    return DAC_Start(dac, WAVEFORM_DAC_CHANNEL) == HAL_OK;
}
