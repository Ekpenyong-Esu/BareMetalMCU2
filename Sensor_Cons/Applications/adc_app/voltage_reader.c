/**
 * @file voltage_reader.c
 * @brief SRP module: owns the ADC scan sequence and turns it into voltages.
 *
 * The ADC converts one channel at a time; a scan sequence just tells it which
 * channels to visit and in what order. Each channel gets its own sampling
 * time, because how long the sample capacitor needs to charge depends on the
 * source impedance, not on the ADC.
 */

#include "voltage_reader.h"

#include "adc_channels.h"
#include "adc_clock.h"
#include "adc_convert.h"
#include "adc_core.h"
#include "adc_measure.h"

/* Scan order, one entry per rank. VREFINT is the internal 1.21 V reference:
 * it needs no wiring and doubles as a sanity check on the 3.3 V supply. */
static const uint32_t kChannels[VOLTAGE_READER_CHANNEL_COUNT] = {
    ADC_CHANNEL_0,        /* PA0 */
    ADC_CHANNEL_1,        /* PA1 */
    ADC_CHANNEL_2,        /* PA2 */
    ADC_CHANNEL_VREFINT,
};

/* Datasheet minimum sampling time for the internal reference. */
#define VREFINT_MIN_SAMPLING_US  10u

bool VoltageReader_Init(VoltageReader_t *reader)
{
    const ADC_ConfigTypeDef adcConfig = {
        .instance      = ADC1,
        .channel       = kChannels[0],
        .resolution    = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_56CYCLES,
        .conv_mode     = ADC_MODE_SINGLE,
        .dma_enabled   = false,
    };

    if (ADC_Init(&reader->adc, &adcConfig) != HAL_OK) {
        return false;
    }

    uint32_t samplingTimes[VOLTAGE_READER_CHANNEL_COUNT];
    for (uint32_t i = 0; i < VOLTAGE_READER_CHANNEL_COUNT; i++) {
        samplingTimes[i] = ADC_SAMPLETIME_56CYCLES;
    }

    /* Stated in microseconds and translated to cycles, so it survives a
     * change to the clock tree. */
    samplingTimes[VOLTAGE_READER_CHANNEL_COUNT - 1u] =
        ADC_Clock_SamplingTimeFor(ADC1, VREFINT_MIN_SAMPLING_US);

    return ADC_ConfigMultiChannel(&reader->adc, kChannels, samplingTimes,
                                  VOLTAGE_READER_CHANNEL_COUNT) == HAL_OK;
}

bool VoltageReader_Read(VoltageReader_t *reader, float *volts)
{
    if (ADC_StartConversion(&reader->adc) != HAL_OK) {
        return false;
    }

    /* One wait per rank: end-of-conversion is raised after every channel, and
     * reading the data register clears it and releases the next conversion.
     * The CPU spins here for the whole sequence. */
    for (uint32_t i = 0; i < VOLTAGE_READER_CHANNEL_COUNT; i++) {
        uint32_t raw = 0;

        if (ADC_PollForConversion(&reader->adc, ADC_CONVERSION_TIMEOUT) != HAL_OK ||
            ADC_GetValue(&reader->adc, &raw) != HAL_OK) {
            return false;
        }

        volts[i] = ADC_RawToVoltage(raw, reader->adc.config.resolution);
    }

    return true;
}

const char *VoltageReader_ChannelName(uint32_t index)
{
    if (index >= VOLTAGE_READER_CHANNEL_COUNT) {
        return "----";
    }
    return ADC_GetChannelName(kChannels[index]);
}
