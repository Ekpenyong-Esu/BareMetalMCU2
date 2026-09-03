/**
 * @file dac_convert.c
 * @brief Conversion between DAC codes and volts implementation
 *
 * Voltage Conversion:
 * - DAC is 12-bit: 0..4095 codes
 * - Reference voltage = VDDA (typically 3.3V on Discovery board)
 * - V_out = (code / 4095) * VREF+
 * - Code = round(voltage / VREF+ * 4095)
 *
 * These functions use DAC_REFERENCE_VOLTAGE (3.3V) as the reference.
 * For precise applications, measure actual VDDA and adjust.
 */

#include "dac_convert.h"

/**
 * @brief Convert a DAC code to its nominal output voltage
 *
 * Formula: voltage = (raw_value / 4095) * DAC_REFERENCE_VOLTAGE
 * Input is clamped to 0..DAC_MAX_VALUE_12BIT.
 *
 * @param raw_value Code, clamped to DAC_MAX_VALUE_12BIT
 * @retval float Voltage in volts (0..DAC_REFERENCE_VOLTAGE)
 */
float DAC_RawToVoltage(uint32_t raw_value)
{
    if (raw_value > DAC_MAX_VALUE_12BIT) {
        raw_value = DAC_MAX_VALUE_12BIT;
    }

    return ((float)raw_value * DAC_REFERENCE_VOLTAGE) / (float)DAC_MAX_VALUE_12BIT;
}

/**
 * @brief Convert a voltage to the nearest DAC code
 *
 * Formula: code = round(voltage / DAC_REFERENCE_VOLTAGE * 4095)
 * Input voltage is clamped to 0..DAC_REFERENCE_VOLTAGE.
 * Result is clamped to 0..DAC_MAX_VALUE_12BIT.
 * Uses rounding (+0.5f) rather than truncation to avoid half-LSB bias.
 *
 * @param voltage Voltage in volts, clamped to the reference range
 * @retval uint32_t Code from 0 to DAC_MAX_VALUE_12BIT
 */
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
