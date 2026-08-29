/**
 * @file voltage_reader.c
 * @brief SRP module: owns the ADC scan sequence and turns it into voltages.
 *
 * The ADC converts one channel at a time; a scan sequence just tells it which
 * channels to visit and in what order. Each channel gets its own sampling
 * time, because how long the sample capacitor needs to charge depends on the
 * source impedance, not on the ADC.
 *
 * Every conversion raises a DMA request, so the results are written to memory
 * by the DMA controller in rank order. The CPU is out of the sample path
 * entirely and only hears about the scan once, when the transfer completes.
 */

#include "voltage_reader.h"

#include "adc_channels.h"
#include "adc_clock.h"
#include "adc_convert.h"
#include "adc_core.h"
#include "adc_measure.h"

#include <stddef.h>

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

/* ADC1 is served by DMA2 Stream 0, Channel 0; see Peripherals/ADC/adc_hw.c. */
#define VOLTAGE_READER_DMA_IRQN      DMA2_Stream0_IRQn
#define VOLTAGE_READER_IRQ_PRIORITY  5u

/* The driver callback carries the ADC handle, not the reader that owns it. */
static VoltageReader_t *s_reader;

/* ISR context: called once per scan, when the last rank has been stored. */
static void OnScanComplete(ADC_HandleStruct *adc, uint32_t value)
{
    (void)adc;
    (void)value; /* the samples are already in reader->raw */

    if (s_reader != NULL) {
        s_reader->complete = true;
    }
}

bool VoltageReader_Init(VoltageReader_t *reader)
{
    const ADC_ConfigTypeDef adcConfig = {
        .instance      = ADC1,
        .channel       = kChannels[0],
        .resolution    = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_56CYCLES,
        .conv_mode     = ADC_MODE_SINGLE,
        .dma_enabled   = true,
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

    if (ADC_ConfigMultiChannel(&reader->adc, kChannels, samplingTimes,
                               VOLTAGE_READER_CHANNEL_COUNT) != HAL_OK) {
        return false;
    }

    s_reader = reader;
    ADC_RegisterConvCompleteCallback(&reader->adc, OnScanComplete);

    HAL_NVIC_SetPriority(VOLTAGE_READER_DMA_IRQN, VOLTAGE_READER_IRQ_PRIORITY, 0u);
    HAL_NVIC_EnableIRQ(VOLTAGE_READER_DMA_IRQN);

    return true;
}

bool VoltageReader_Start(VoltageReader_t *reader)
{
    reader->complete = false;

    return ADC_StartDMA(&reader->adc, reader->raw,
                        VOLTAGE_READER_CHANNEL_COUNT) == HAL_OK;
}

bool VoltageReader_IsComplete(const VoltageReader_t *reader)
{
    return reader->complete;
}

bool VoltageReader_Take(VoltageReader_t *reader, float *volts)
{
    if (!reader->complete) {
        return false;
    }

    /* Releases the stream so the next Start() can rearm it. */
    (void)ADC_StopDMA(&reader->adc);

    /* Scale against the supply this scan actually ran on, not a nominal 3.3 V. */
    reader->vdda = ADC_MeasureVdda(reader->raw[VOLTAGE_READER_VREFINT_INDEX],
                                   reader->adc.config.resolution);

    for (uint32_t i = 0; i < VOLTAGE_READER_CHANNEL_COUNT; i++) {
        volts[i] = ADC_RawToVoltageRef(reader->raw[i], reader->adc.config.resolution,
                                       reader->vdda);
    }

    return true;
}

float VoltageReader_Vdda(const VoltageReader_t *reader)
{
    return reader->vdda;
}

const char *VoltageReader_ChannelName(uint32_t index)
{
    if (index >= VOLTAGE_READER_CHANNEL_COUNT) {
        return "----";
    }
    return ADC_GetChannelName(kChannels[index]);
}
