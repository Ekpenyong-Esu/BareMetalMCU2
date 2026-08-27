/**
 * @file voltage_reader.c
 * @brief SRP module: owns the ADC channel and turns it into voltage samples.
 */

#include "voltage_reader.h"

#include "adc_core.h"
#include "adc_measure.h"

#define VOLTAGE_READER_CHANNEL  ADC_CHANNEL_0  /* PA0 on the discovery header */

bool VoltageReader_Init(VoltageReader_t *reader)
{
    const ADC_ConfigTypeDef adcConfig = {
        .instance      = ADC1,
        .channel       = VOLTAGE_READER_CHANNEL,
        .resolution    = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_56CYCLES,
        .conv_mode     = ADC_MODE_SINGLE,
        .dma_enabled   = false,
    };

    return ADC_Init(&reader->adc, &adcConfig) == HAL_OK;
}

float VoltageReader_Read(VoltageReader_t *reader)
{
    return ADC_ReadChannelVoltage(&reader->adc, VOLTAGE_READER_CHANNEL);
}
