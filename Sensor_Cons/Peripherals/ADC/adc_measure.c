/**
 * @file    adc_measure.c
 * @brief   Raw count conversion and internal sensor implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "adc_measure.h"

#include "adc_convert.h"

/* Private constants ---------------------------------------------------------*/

#define ADC_TEMP_V25        0.76f       /*!< Sensor voltage at 25 degrees C */
#define ADC_TEMP_AVG_SLOPE  0.0025f     /*!< Sensor slope in V per degree C */
#define ADC_TEMP_25C        25.0f       /*!< Reference temperature */
#define ADC_VBAT_DIVIDER    2.0f        /*!< VBAT internal divider ratio */
#define ADC_ABSOLUTE_ZERO   -273.15f    /*!< Returned when a temperature read fails */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Read one channel and convert the raw count to volts
 * @param   hadc ADC handle
 * @param   channel Channel to read
 * @param   onError Value returned when the read fails
 * @retval  float Voltage in Volts, or @p onError on failure
 */
static float ADC_ReadScaled(ADC_HandleStruct* hadc, uint32_t channel, float onError)
{
    uint32_t raw = 0;

    if (ADC_ReadChannel(hadc, channel, &raw) != HAL_OK) {
        return onError;
    }

    return ADC_RawToVoltage(raw, hadc->config.resolution);
}

/* Exported functions --------------------------------------------------------*/

/* Full-scale count depends on the configured resolution (12-bit -> 4095). */
uint32_t ADC_GetMaxValue(uint32_t resolution)
{
    switch (resolution) {
        case ADC_RESOLUTION_12B: return ADC_MAX_VALUE_12BIT;
        case ADC_RESOLUTION_10B: return ADC_MAX_VALUE_10BIT;
        case ADC_RESOLUTION_8B:  return ADC_MAX_VALUE_8BIT;
        case ADC_RESOLUTION_6B:  return ADC_MAX_VALUE_6BIT;
        default:                 return ADC_MAX_VALUE_12BIT;
    }
}

/* Linear scale: raw / max * 3.3 V. */
float ADC_RawToVoltage(uint32_t raw_value, uint32_t resolution)
{
    return ((float)raw_value * ADC_REFERENCE_VOLTAGE) / (float)ADC_GetMaxValue(resolution);
}

/* Inverse of ADC_RawToVoltage. */
uint32_t ADC_VoltageToRaw(float voltage, uint32_t resolution)
{
    return (uint32_t)((voltage * (float)ADC_GetMaxValue(resolution)) / ADC_REFERENCE_VOLTAGE);
}

float ADC_ReadChannelVoltage(ADC_HandleStruct* hadc, uint32_t channel)
{
    return ADC_ReadScaled(hadc, channel, -1.0f);
}

float ADC_ReadTemperature(ADC_HandleStruct* hadc)
{
    float voltage = ADC_ReadScaled(hadc, ADC_CHANNEL_TEMPSENSOR, ADC_ABSOLUTE_ZERO);
    if (voltage == ADC_ABSOLUTE_ZERO) {
        return ADC_ABSOLUTE_ZERO;
    }

    /* Reference manual: Temp = (VSENSE - V25) / Avg_Slope + 25 */
    return ((voltage - ADC_TEMP_V25) / ADC_TEMP_AVG_SLOPE) + ADC_TEMP_25C;
}

float ADC_ReadVrefInt(ADC_HandleStruct* hadc)
{
    return ADC_ReadScaled(hadc, ADC_CHANNEL_VREFINT, -1.0f);
}

/* VBAT reaches the ADC through an internal /2 divider, so double the result. */
float ADC_ReadVbat(ADC_HandleStruct* hadc)
{
    float voltage = ADC_ReadScaled(hadc, ADC_CHANNEL_VBAT, -1.0f);
    if (voltage < 0.0f) {
        return -1.0f;
    }

    return voltage * ADC_VBAT_DIVIDER;
}
