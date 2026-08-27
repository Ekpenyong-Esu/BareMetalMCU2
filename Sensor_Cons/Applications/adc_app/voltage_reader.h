/**
 * @file voltage_reader.h
 * @brief SRP module: owns the ADC channel and turns it into voltage samples.
 */

#ifndef VOLTAGE_READER_H
#define VOLTAGE_READER_H

#include "adc_types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ADC_HandleStruct adc;
} VoltageReader_t;

/** Bring up ADC1 channel 0 (PA0) for single-conversion polling reads. */
bool VoltageReader_Init(VoltageReader_t *reader);

/** Take one reading; returns the voltage in volts, or a negative value on error. */
float VoltageReader_Read(VoltageReader_t *reader);

#ifdef __cplusplus
}
#endif

#endif /* VOLTAGE_READER_H */
