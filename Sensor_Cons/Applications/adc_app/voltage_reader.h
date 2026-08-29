/**
 * @file voltage_reader.h
 * @brief SRP module: owns the ADC scan sequence and turns it into voltages.
 */

#ifndef VOLTAGE_READER_H
#define VOLTAGE_READER_H

#include "adc_types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Channels in the scan; also the length of the buffer Read() fills. */
#define VOLTAGE_READER_CHANNEL_COUNT  4u

typedef struct {
    ADC_HandleStruct adc;
} VoltageReader_t;

/** Bring up ADC1 in scan mode over the channel list. */
bool VoltageReader_Init(VoltageReader_t *reader);

/** Run one scan; fills VOLTAGE_READER_CHANNEL_COUNT volts. False on error. */
bool VoltageReader_Read(VoltageReader_t *reader, float *volts);

/** Pin name of the channel at @p index in the scan, for labelling output. */
const char *VoltageReader_ChannelName(uint32_t index);

#ifdef __cplusplus
}
#endif

#endif /* VOLTAGE_READER_H */
