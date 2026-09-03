/**
 * @file dac_convert.h
 * @brief Conversion between DAC codes and volts
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

#ifndef DAC_CONVERT_H
#define DAC_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dac_types.h"

/**
 * @brief   Convert a DAC code to its nominal output voltage
 *
 * Formula: voltage = (raw_value / 4095) * DAC_REFERENCE_VOLTAGE
 * Input is clamped to 0..DAC_MAX_VALUE_12BIT.
 *
 * @param   raw_value Code, clamped to DAC_MAX_VALUE_12BIT
 * @retval  float Voltage in volts (0..DAC_REFERENCE_VOLTAGE)
 */
float DAC_RawToVoltage(uint32_t raw_value);

/**
 * @brief   Convert a voltage to the nearest DAC code
 *
 * Formula: code = round(voltage / DAC_REFERENCE_VOLTAGE * 4095)
 * Input voltage is clamped to 0..DAC_REFERENCE_VOLTAGE.
 * Result is clamped to 0..DAC_MAX_VALUE_12BIT.
 *
 * @param   voltage Voltage in volts, clamped to the reference range
 * @retval  uint32_t Code from 0 to DAC_MAX_VALUE_12BIT
 */
uint32_t DAC_VoltageToRaw(float voltage);

#ifdef __cplusplus
}
#endif

#endif /* DAC_CONVERT_H */
