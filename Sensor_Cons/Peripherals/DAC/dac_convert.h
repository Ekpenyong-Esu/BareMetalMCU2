/**
 * @file dac_convert.h
 * @brief Conversion between DAC codes and volts
 */

#ifndef DAC_CONVERT_H
#define DAC_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dac_types.h"

/**
 * @brief   Convert a DAC code to its nominal output voltage
 * @param   raw_value Code, clamped to DAC_MAX_VALUE_12BIT
 * @retval  float Voltage in volts
 */
float DAC_RawToVoltage(uint32_t raw_value);

/**
 * @brief   Convert a voltage to the nearest DAC code
 * @param   voltage Voltage in volts, clamped to the reference range
 * @retval  uint32_t Code from 0 to DAC_MAX_VALUE_12BIT
 */
uint32_t DAC_VoltageToRaw(float voltage);

#ifdef __cplusplus
}
#endif

#endif /* DAC_CONVERT_H */
