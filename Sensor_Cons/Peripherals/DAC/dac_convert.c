/**
 * @file dac_convert.c
 * @brief Conversion between DAC codes and volts
 */

#include "dac_convert.h"

float DAC_RawToVoltage(uint32_t raw_value)
{
    if (raw_value > DAC_MAX_VALUE_12BIT) {
        raw_value = DAC_MAX_VALUE_12BIT;
    }

    return ((float)raw_value * DAC_REFERENCE_VOLTAGE) / (float)DAC_MAX_VALUE_12BIT;
}

uint32_t DAC_VoltageToRaw(float voltage)
{
    float code;

    if (voltage < 0.0f) {
        voltage = 0.0f;
    }
    if (voltage > DAC_REFERENCE_VOLTAGE) {
        voltage = DAC_REFERENCE_VOLTAGE;
    }

    /* Round rather than truncate; half a LSB of bias is otherwise added. */
    code = ((voltage * (float)DAC_MAX_VALUE_12BIT) / DAC_REFERENCE_VOLTAGE) + 0.5f;

    return (uint32_t)code;
}
