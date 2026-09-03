/**
 * @file    adc_measure.h
 * @brief   Raw count to physical unit conversion and internal sensors
 * @details Pure scaling maths plus the three on-die channels. The only layer
 *          that knows about the reference voltage and the temperature curve.
 *
 * Voltage Scaling:
 * - ADC measures ratio: V_in / VDDA = raw / max_value
 * - VDDA is assumed 3.3V nominal, but can be calibrated via VREFINT
 * - ADC_CalibrateVdda() reads VREFINT (1.21V typical) and computes actual VDDA
 * - After calibration, all voltage conversions use the measured VDDA
 *
 * Temperature Sensor:
 * - STM32F4 has internal temperature sensor on ADC_CHANNEL_TEMPSENSOR (CH16)
 * - Formula: Temp = ((V_sense - V25) / Avg_Slope) + 25
 * - V25 = 0.76V typical, Avg_Slope = 2.5mV/°C typical (datasheet values)
 * - Requires VDDA calibration for accuracy
 *
 * VREFINT:
 * - Internal 1.21V bandgap reference on ADC_CHANNEL_VREFINT (CH17)
 * - Used to calibrate VDDA: VDDA = VREFINT_TYPICAL * max_value / raw_vrefint
 *
 * VBAT:
 * - Battery voltage / 2 on ADC_CHANNEL_VBAT (CH18)
 * - Requires VDDA calibration for accuracy
 */

#ifndef ADC_MEASURE_H
#define ADC_MEASURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Full-scale count for a resolution
 *
 * @param   resolution HAL resolution constant (ADC_RESOLUTION_12B, 10B, 8B, 6B)
 * @retval  uint32_t Maximum raw value (4095, 1023, 255, 63)
 */
uint32_t ADC_GetMaxValue(uint32_t resolution);

/**
 * @brief   Convert a raw count to volts
 *
 * Uses the handle's resolution and calibrated VDDA:
 *   voltage = raw_value / max_value * vdda
 *
 * @param   hadc ADC handle supplying the resolution and the reference supply
 * @param   raw_value Raw ADC value (0..max_value)
 * @retval  float Voltage in Volts
 */
float ADC_RawToVoltage(const ADC_HandleStruct* hadc, uint32_t raw_value);

/**
 * @brief   Convert volts to a raw count
 *
 * Inverse of ADC_RawToVoltage():
 *   raw = voltage / vdda * max_value
 *
 * @param   hadc ADC handle supplying the resolution and the reference supply
 * @param   voltage Voltage in Volts
 * @retval  uint32_t Raw ADC value (clamped to max_value)
 */
uint32_t ADC_VoltageToRaw(const ADC_HandleStruct* hadc, float voltage);

/**
 * @brief   Measure the real supply through VREFINT and scale against it from now on
 *
 * Counts are a ratio of VDDA, not of 3.3V. This reads the on-die reference
 * (VREFINT, 1.21V typical) and works VDDA back out:
 *   VDDA = VREFINT_TYPICAL * max_value / raw_vrefint
 *
 * Updates hadc->vdda for all subsequent voltage conversions.
 * Should be called after ADC initialization and before any voltage measurements.
 *
 * @param   hadc ADC handle (must be initialized, VREFINT channel accessible)
 * @retval  HAL_StatusTypeDef HAL_OK when hadc->vdda was updated, HAL_ERROR on failure
 */
HAL_StatusTypeDef ADC_CalibrateVdda(ADC_HandleStruct* hadc);

/**
 * @brief   Read a channel and return volts
 *
 * Convenience: ADC_ReadChannel() + ADC_RawToVoltage().
 *
 * @param   hadc ADC handle
 * @param   channel Channel to read
 * @retval  float Voltage in Volts, or -1.0f on error
 */
float ADC_ReadChannelVoltage(ADC_HandleStruct* hadc, uint32_t channel);

/**
 * @brief   Read the on-die temperature sensor
 *
 * Reads ADC_CHANNEL_TEMPSENSOR and converts using STM32F4 formula:
 *   Temp = ((V_sense - V25) / Avg_Slope) + 25
 * where V25 = 0.76V, Avg_Slope = 2.5mV/°C (datasheet typical values).
 * Requires prior ADC_CalibrateVdda() for accuracy.
 *
 * @param   hadc ADC handle
 * @retval  float Temperature in Celsius, or -273.15f (absolute zero) on error
 */
float ADC_ReadTemperature(ADC_HandleStruct* hadc);

/**
 * @brief   Read the internal voltage reference
 *
 * Reads ADC_CHANNEL_VREFINT. Useful for monitoring VDDA stability.
 *
 * @param   hadc ADC handle
 * @retval  float Voltage in Volts (should be ~1.21V), or -1.0f on error
 */
float ADC_ReadVrefInt(ADC_HandleStruct* hadc);

/**
 * @brief   Read the battery pin through its internal divider
 *
 * Reads ADC_CHANNEL_VBAT (VBAT/2). Multiplies by 2 to get actual VBAT.
 * Requires prior ADC_CalibrateVdda() for accuracy.
 *
 * @param   hadc ADC handle
 * @retval  float Voltage in Volts, or -1.0f on error
 */
float ADC_ReadVbat(ADC_HandleStruct* hadc);

#ifdef __cplusplus
}
#endif

#endif /* ADC_MEASURE_H */
