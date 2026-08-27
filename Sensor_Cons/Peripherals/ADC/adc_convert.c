/**
 * @file    adc_convert.c
 * @brief   ADC conversion control implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "adc_convert.h"

#include "adc_core.h"

#include <stddef.h>

/* Exported functions --------------------------------------------------------*/

/* Triggers one software-start conversion; caller polls or waits for it. */
HAL_StatusTypeDef ADC_StartConversion(ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Start(&hadc->hal_handle);
}

/* Blocks until end-of-conversion or the timeout expires. */
HAL_StatusTypeDef ADC_PollForConversion(ADC_HandleStruct* hadc, uint32_t timeout_ms)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_PollForConversion(&hadc->hal_handle, timeout_ms);
}

/* Reads the data register; only meaningful right after a completed conversion. */
HAL_StatusTypeDef ADC_GetValue(ADC_HandleStruct* hadc, uint32_t* value)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    if (value == NULL) {
        return HAL_ERROR;
    }

    *value = HAL_ADC_GetValue(&hadc->hal_handle);

    return HAL_OK;
}

/* One-shot read: configure the channel, start, wait, return the raw count.
 * This is the simplest way to use the ADC and what the voltmeter app uses. */
HAL_StatusTypeDef ADC_ReadChannel(ADC_HandleStruct* hadc, uint32_t channel,
                                  uint32_t* value)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    if (value == NULL) {
        return HAL_ERROR;
    }

    status = ADC_ConfigChannel(hadc, channel, ADC_SAMPLETIME_84CYCLES);
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

    return ADC_GetValue(hadc, value);
}

HAL_StatusTypeDef ADC_StartContinuousConversion(ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    /* Flips the mode without a full HAL_ADC_Init(); HAL_ADC_Start() alone picks it up. */
    hadc->hal_handle.Init.ContinuousConvMode = ENABLE;

    return HAL_ADC_Start(&hadc->hal_handle);
}

/* Counterpart to ADC_StartContinuousConversion(); leaves the mode flag set. */
HAL_StatusTypeDef ADC_StopContinuousConversion(ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Stop(&hadc->hal_handle);
}

/* Streams @p length conversions into @p buffer without CPU involvement per sample. */
HAL_StatusTypeDef ADC_StartDMA(ADC_HandleStruct* hadc, uint32_t* buffer, uint32_t length)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    if (buffer == NULL || length == 0) {
        return HAL_ERROR;
    }

    return HAL_ADC_Start_DMA(&hadc->hal_handle, buffer, length);
}

/* Counterpart to ADC_StartDMA(); the buffer keeps whatever it last received. */
HAL_StatusTypeDef ADC_StopDMA(ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Stop_DMA(&hadc->hal_handle);
}

/* DMA plus polling: start the stream, wait for the sequence to finish, then
 * the caller reads the results out of the buffer it passed in. */
HAL_StatusTypeDef ADC_ReadMultiChannel(ADC_HandleStruct* hadc, uint32_t* values,
                                       uint32_t num_channels)
{
    HAL_StatusTypeDef status = ADC_StartDMA(hadc, values, num_channels);
    if (status != HAL_OK) {
        return status;
    }

    status = ADC_PollForConversion(hadc, ADC_CONVERSION_TIMEOUT);
    if (status != HAL_OK) {
        (void)ADC_StopDMA(hadc);
        return status;
    }

    return HAL_OK;
}

HAL_StatusTypeDef ADC_Start_IT(ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    /* Cleared here; the conversion-complete ISR sets it again. */
    hadc->conv_complete_flag = false;

    return HAL_ADC_Start_IT(&hadc->hal_handle);
}

/* Counterpart to ADC_Start_IT(); the conversion-complete callback won't fire again. */
HAL_StatusTypeDef ADC_Stop_IT(ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Stop_IT(&hadc->hal_handle);
}

/* Non-blocking counterpart to ADC_ReadChannel(): result arrives later via the
 * conversion-complete callback instead of a polled wait. */
HAL_StatusTypeDef ADC_ReadChannel_IT(ADC_HandleStruct* hadc, uint32_t channel)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    status = ADC_ConfigChannel(hadc, channel, ADC_SAMPLETIME_84CYCLES);
    if (status != HAL_OK) {
        return status;
    }

    return ADC_Start_IT(hadc);
}

/* Result delivered via this callback from ISR context on every conversion. */
void ADC_RegisterConvCompleteCallback(ADC_HandleStruct* hadc,
                                      void (*callback)(ADC_HandleStruct*, uint32_t))
{
    if (hadc != NULL) {
        hadc->conv_complete_cb = callback;
    }
}

/* Invoked from ISR context on overrun/DMA errors; see HAL_ADC_ErrorCallback(). */
void ADC_RegisterErrorCallback(ADC_HandleStruct* hadc,
                               void (*callback)(ADC_HandleStruct*))
{
    if (hadc != NULL) {
        hadc->error_cb = callback;
    }
}
