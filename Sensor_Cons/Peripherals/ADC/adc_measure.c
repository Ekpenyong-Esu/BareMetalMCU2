/**
 * @file    adc_measure.c
 * @brief   Raw count conversion and internal sensor implementation
 *
 * This module implements the physical unit conversions:
 * - Raw ADC count <-> Voltage (using calibrated VDDA)
 * - VDDA calibration via VREFINT (internal 1.21V bandgap reference)
 * - Temperature sensor reading (STM32F4 internal sensor)
 * - VREFINT reading (for monitoring VDDA)
 * - VBAT reading (battery voltage through internal divider)
 *
 * Key Formulas:
 * - Voltage = raw / max_value * VDDA
 * - VDDA = VREFINT_TYPICAL * max_value / raw_vrefint
 * - Temperature = ((V_sense - V25) / Avg_Slope) + 25
 *   where V25 = 0.76V, Avg_Slope = 2.5mV/°C (datasheet typical)
 * - VBAT = 2 * V_bat_channel (internal divider is /2)
 *
 * Internal channels (VREFINT, TEMP, VBAT) have high source impedance
 * and require long sampling time (480 cycles) for accurate readings.
 */

#include "adc_measure.h"

#include "adc_convert.h"
#include "adc_core.h"

/* Private constants ---------------------------------------------------------*/

#define ADC_TEMP_V25 0.76f           /*!< Sensor voltage at 25 degrees C */
#define ADC_TEMP_AVG_SLOPE 0.0025f   /*!< Sensor slope in V per degree C */
#define ADC_TEMP_25C 25.0f           /*!< Reference temperature */
#define ADC_VBAT_DIVIDER 2.0f        /*!< VBAT internal divider ratio */
#define ADC_ABSOLUTE_ZERO (-273.15f) /*!< Returned when a temperature read fails */

/** The datasheet asks for at least 10 us on VREFINT and the temperature sensor. */
#define ADC_INTERNAL_SAMPLETIME ADC_SAMPLETIME_480CYCLES

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Read an on-die channel at the long sample time its source impedance needs
 *
 * Internal channels (VREFINT, TEMP, VBAT) have high output impedance and
 * require long sampling time (480 cycles = ~10+ µs at typical ADC clock).
 *
 * @param hadc ADC handle
 * @param channel Internal channel (ADC_CHANNEL_VREFINT, TEMPSENSOR, VBAT)
 * @param raw Out: raw ADC value
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADC_ReadInternalRaw(ADC_HandleStruct *hadc, uint32_t channel,
                                             uint32_t *raw) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    status = ADC_ConfigChannel(hadc, channel, ADC_INTERNAL_SAMPLETIME);
    if (status != HAL_OK) {
        return status;
    }

    status = ADC_StartConversion(hadc);
    if (status != HAL_OK) {
        return status;
    }

    status = ADC_PollForConversion(hadc, ADC_CONVERSION_TIMEOUT);
    if (status != HAL_OK) {
        return status;
    }

    return ADC_GetValue(hadc, raw);
}

/**
 * @brief Read an on-die channel and scale it, returning @p onError on failure
 *
 * @param hadc ADC handle
 * @param channel Internal channel
 * @param onError Value to return on failure
 * @retval float Scaled voltage, or onError
 */
static float ADC_ReadInternalScaled(ADC_HandleStruct *hadc, uint32_t channel, float onError) {
    uint32_t raw = 0;

    if (ADC_ReadInternalRaw(hadc, channel, &raw) != HAL_OK) {
        return onError;
    }

    return ADC_RawToVoltage(hadc, raw);
}

/**
 * @brief Read one channel and scale it, returning @p onError on failure
 *
 * @param hadc ADC handle
 * @param channel External channel
 * @param onError Value to return on failure
 * @retval float Scaled voltage, or onError
 */
static float ADC_ReadScaled(ADC_HandleStruct *hadc, uint32_t channel, float onError) {
    uint32_t raw = 0;

    if (ADC_ReadChannel(hadc, channel, &raw) != HAL_OK) {
        return onError;
    }

    return ADC_RawToVoltage(hadc, raw);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Full-scale count for a resolution
 *
 * @param resolution HAL resolution constant (ADC_RESOLUTION_12B, 10B, 8B, 6B)
 * @retval uint32_t Maximum raw value (4095, 1023, 255, 63)
 */
uint32_t ADC_GetMaxValue(uint32_t resolution) {
    switch (resolution) {
        case ADC_RESOLUTION_12B:
            return ADC_MAX_VALUE_12BIT;
        case ADC_RESOLUTION_10B:
            return ADC_MAX_VALUE_10BIT;
        case ADC_RESOLUTION_8B:
            return ADC_MAX_VALUE_8BIT;
        case ADC_RESOLUTION_6B:
            return ADC_MAX_VALUE_6BIT;
        default:
            return ADC_MAX_VALUE_12BIT;
    }
}

/**
 * @brief Convert a raw count to volts
 *
 * Uses the handle's resolution and calibrated VDDA:
 *   voltage = raw_value / max_value * vdda
 *
 * @param hadc ADC handle supplying the resolution and the reference supply
 * @param raw_value Raw ADC value (0..max_value)
 * @retval float Voltage in Volts
 */
float ADC_RawToVoltage(const ADC_HandleStruct *hadc, uint32_t raw_value) {
    if (hadc == NULL) {
        return 0.0f;
    }

    return ((float)raw_value * hadc->vdda) / (float)ADC_GetMaxValue(hadc->config.resolution);
}

/**
 * @brief Convert volts to a raw count
 *
 * Inverse of ADC_RawToVoltage():
 *   raw = voltage / vdda * max_value
 *
 * @param hadc ADC handle supplying the resolution and the reference supply
 * @param voltage Voltage in Volts
 * @retval uint32_t Raw ADC value (clamped to max_value)
 */
uint32_t ADC_VoltageToRaw(const ADC_HandleStruct *hadc, float voltage) {
    if (hadc == NULL || hadc->vdda <= 0.0f) {
        return 0U;
    }

    return (uint32_t)((voltage * (float)ADC_GetMaxValue(hadc->config.resolution)) / hadc->vdda);
}

/**
 * @brief Measure the real supply through VREFINT and scale against it from now on
 *
 * Counts are a ratio of VDDA, not of 3.3V. This reads the on-die reference
 * (VREFINT, 1.21V typical) and works VDDA back out:
 *   VDDA = VREFINT_TYPICAL * max_value / raw_vrefint
 *
 * Updates hadc->vdda for all subsequent voltage conversions.
 * Should be called after ADC initialization and before any voltage measurements.
 *
 * @param hadc ADC handle (must be initialized, VREFINT channel accessible)
 * @retval HAL_StatusTypeDef HAL_OK when hadc->vdda was updated, HAL_ERROR on failure
 */
HAL_StatusTypeDef ADC_CalibrateVdda(ADC_HandleStruct *hadc) {
    uint32_t raw = 0;

    HAL_StatusTypeDef status = ADC_ReadInternalRaw(hadc, ADC_CHANNEL_VREFINT, &raw);
    if (status != HAL_OK) {
        return status;
    }

    /* A zero count means VREFINT never switched in; keep the previous scale. */
    if (raw == 0U) {
        return HAL_ERROR;
    }

    hadc->vdda =
        (ADC_VREFINT_TYPICAL * (float)ADC_GetMaxValue(hadc->config.resolution)) / (float)raw;

    return HAL_OK;
}

/**
 * @brief Read a channel and return volts
 *
 * Convenience: ADC_ReadChannel() + ADC_RawToVoltage().
 *
 * @param hadc ADC handle
 * @param channel Channel to read
 * @retval float Voltage in Volts, or -1.0f on error
 */
float ADC_ReadChannelVoltage(ADC_HandleStruct *hadc, uint32_t channel) {
    return ADC_ReadScaled(hadc, channel, -1.0f);
}

/**
 * @brief Read the on-die temperature sensor
 *
 * Reads ADC_CHANNEL_TEMPSENSOR and converts using STM32F4 formula:
 *   Temp = ((V_sense - V25) / Avg_Slope) + 25
 * where V25 = 0.76V, Avg_Slope = 2.5mV/°C (datasheet typical values).
 * Requires prior ADC_CalibrateVdda() for accuracy.
 *
 * @param hadc ADC handle
 * @retval float Temperature in Celsius, or -273.15f (absolute zero) on error
 */
float ADC_ReadTemperature(ADC_HandleStruct *hadc) {
    float voltage = ADC_ReadInternalScaled(hadc, ADC_CHANNEL_TEMPSENSOR, ADC_ABSOLUTE_ZERO);
    if (voltage == ADC_ABSOLUTE_ZERO) {
        return ADC_ABSOLUTE_ZERO;
    }

    /* Reference manual: Temp = (VSENSE - V25) / Avg_Slope + 25 */
    return ((voltage - ADC_TEMP_V25) / ADC_TEMP_AVG_SLOPE) + ADC_TEMP_25C;
}

/**
 * @brief Read the internal voltage reference
 *
 * Reads ADC_CHANNEL_VREFINT. Useful for monitoring VDDA stability.
 *
 * @param hadc ADC handle
 * @retval float Voltage in Volts (should be ~1.21V), or -1.0f on error
 */
float ADC_ReadVrefInt(ADC_HandleStruct *hadc) {
    return ADC_ReadInternalScaled(hadc, ADC_CHANNEL_VREFINT, -1.0f);
}

/**
 * @brief Read the battery pin through its internal divider
 *
 * Reads ADC_CHANNEL_VBAT (VBAT/2). Multiplies by 2 to get actual VBAT.
 * Requires prior ADC_CalibrateVdda() for accuracy.
 *
 * @param hadc ADC handle
 * @retval float Voltage in Volts, or -1.0f on error
 */
float ADC_ReadVbat(ADC_HandleStruct *hadc) {
    float voltage = ADC_ReadInternalScaled(hadc, ADC_CHANNEL_VBAT, -1.0f);
    if (voltage < 0.0f) {
        return -1.0f;
    }

    return voltage * ADC_VBAT_DIVIDER;
}
