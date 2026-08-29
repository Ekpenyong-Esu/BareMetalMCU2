/**
 * @file voltage_reader.h
 * @brief SRP module: owns the ADC scan sequence and turns it into voltages.
 *
 * DMA-driven: starting a scan returns immediately and the results are written
 * straight into the buffer below without the CPU touching a single sample.
 */

#ifndef VOLTAGE_READER_H
#define VOLTAGE_READER_H

#include "adc_types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Channels in the scan; also the length of the buffer Take() fills. */
#define VOLTAGE_READER_CHANNEL_COUNT  4u

typedef struct {
    ADC_HandleStruct adc;
    /** Written by the DMA controller, so it must outlive every transfer. */
    uint32_t         raw[VOLTAGE_READER_CHANNEL_COUNT];
    volatile bool    complete; /*!< Set by the transfer-complete ISR */
} VoltageReader_t;

/** Bring up ADC1 in scan mode over DMA and enable the stream interrupt. */
bool VoltageReader_Init(VoltageReader_t *reader);

/** Kick off a scan and return; the DMA fills the buffer in the background. */
bool VoltageReader_Start(VoltageReader_t *reader);

/** Whether the scan started by Start() has finished. */
bool VoltageReader_IsComplete(const VoltageReader_t *reader);

/** Convert the finished scan to volts. False if it is not complete yet. */
bool VoltageReader_Take(VoltageReader_t *reader, float *volts);

/** Pin name of the channel at @p index in the scan, for labelling output. */
const char *VoltageReader_ChannelName(uint32_t index);

#ifdef __cplusplus
}
#endif

#endif /* VOLTAGE_READER_H */
