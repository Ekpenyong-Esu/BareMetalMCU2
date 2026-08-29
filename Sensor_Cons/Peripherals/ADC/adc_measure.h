/**
 * @file    adc_measure.h
 * @brief   Raw count to physical unit conversion and internal sensors
 * @details Pure scaling maths plus the three on-die channels. The only layer
 *          that knows about the reference voltage and the temperature curve.
 */

#ifndef ADC_MEASURE_H
#define ADC_MEASURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Full-scale count for a resolution
 * @param   resolution HAL resolution constant
 * @retval  uint32_t Maximum raw value
 */
uint32_t ADC_GetMaxValue(uint32_t resolution);

/**
 * @brief   Convert a raw count to volts
 * @param   raw_value Raw ADC value
 * @param   resolution HAL resolution constant
 * @retval  float Voltage in Volts
 */
float ADC_RawToVoltage(uint32_t raw_value, uint32_t resolution);

/**
 * @brief   Convert a raw count to volts against a known reference
 * @details Counts are a fraction of whatever is on VREF+, so a reading is only
 *          as accurate as the reference it is scaled against. Pair with
 *          ADC_MeasureVdda() instead of trusting the nominal 3.3 V.
 * @param   raw_value Raw ADC value
 * @param   resolution HAL resolution constant
 * @param   reference_voltage Full-scale voltage in Volts
 * @retval  float Voltage in Volts
 */
float ADC_RawToVoltageRef(uint32_t raw_value, uint32_t resolution,
                          float reference_voltage);

/**
 * @brief   Derive the analog supply from a reading of the internal reference
 * @details VREFINT is a bandgap fixed at about 1.21 V, so the count it
 *          produces varies only with VDDA. Every part is factory-trimmed at
 *          exactly 3.3 V and the resulting count is stored in system memory,
 *          which turns one VREFINT conversion into a measurement of the rail.
 * @param   vrefint_raw Raw count from ADC_CHANNEL_VREFINT
 * @param   resolution HAL resolution constant used for that conversion
 * @retval  float VDDA in Volts, or -1.0f when @p vrefint_raw is zero
 */
float ADC_MeasureVdda(uint32_t vrefint_raw, uint32_t resolution);

/**
 * @brief   Convert volts to a raw count
 * @param   voltage Voltage in Volts
 * @param   resolution HAL resolution constant
 * @retval  uint32_t Raw ADC value
 */
uint32_t ADC_VoltageToRaw(float voltage, uint32_t resolution);

/**
 * @brief   Read a channel and return volts
 * @param   hadc ADC handle
 * @param   channel Channel to read
 * @retval  float Voltage in Volts, or -1.0f on error
 */
float ADC_ReadChannelVoltage(ADC_HandleStruct* hadc, uint32_t channel);

/**
 * @brief   Read the on-die temperature sensor
 * @param   hadc ADC handle
 * @retval  float Temperature in Celsius, or absolute zero on error
 */
float ADC_ReadTemperature(ADC_HandleStruct* hadc);

/**
 * @brief   Read the internal voltage reference
 * @param   hadc ADC handle
 * @retval  float Voltage in Volts, or -1.0f on error
 */
float ADC_ReadVrefInt(ADC_HandleStruct* hadc);

/**
 * @brief   Read the battery pin through its internal divider
 * @param   hadc ADC handle
 * @retval  float Voltage in Volts, or -1.0f on error
 */
float ADC_ReadVbat(ADC_HandleStruct* hadc);

/**
 * @brief Convert a raw count to volts
 * @param raw_value Raw ADC value
 * @param resolution HAL resolution constant
 */
#define ADC_RAW_TO_VOLTAGE(raw_value, resolution) \
    ((float)(raw_value) * ADC_REFERENCE_VOLTAGE / (float)(ADC_GetMaxValue(resolution)))

/**
 * @brief Convert volts to a raw count
 * @param voltage Voltage in Volts
 * @param resolution HAL resolution constant
 */
#define ADC_VOLTAGE_TO_RAW(voltage, resolution) \
    ((uint32_t)(((voltage) * (float)(ADC_GetMaxValue(resolution)) / ADC_REFERENCE_VOLTAGE)))

#ifdef __cplusplus
}
#endif

#endif /* ADC_MEASURE_H */
