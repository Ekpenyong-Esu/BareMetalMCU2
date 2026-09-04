/**
 * @file    adc_convert.c
 * @brief   ADC conversion control implementation
 *
 * This module implements all conversion control paths:
 * - Polling (single): Start -> Poll -> GetValue
 * - Continuous: StartContinuous -> GetValue anytime
 * - DMA: StartDMA -> buffer fills in background -> StopDMA
 * - Multi-channel DMA scan: ReadMultiChannel (configured sequence)
 * - Interrupt: Start_IT -> callback on completion
 *
 * Key Implementation Details:
 * - All functions validate handle readiness via ADC_CheckReady()
 * - ADC_ReadChannel() uses a default sampling time (84 cycles) for convenience
 * - ADC_ReadMultiChannel() waits on conv_complete_flag set by DMA callback
 * - Interrupt mode uses conv_complete_cb/error_cb registered by caller
 * - Continuous mode modifies the HAL handle's ContinuousConvMode directly
 */

#include "adc_convert.h"

#include "adc_core.h"

#include <stddef.h>

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Start a software-triggered single conversion
 *
 * Calls HAL_ADC_Start() which enables the ADC and starts a conversion.
 * The conversion runs asynchronously; use ADC_PollForConversion() or
 * ADC_GetValue() to retrieve the result.
 *
 * @param hadc ADC handle (must be initialized)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_StartConversion(ADC_HandleStruct *hadc) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Start(&hadc->hal_handle);
}

/**
 * @brief Block until the current conversion finishes
 *
 * Calls HAL_ADC_PollForConversion() which polls the EOC flag until set
 * or timeout expires. Does not read the data register.
 *
 * @param hadc ADC handle
 * @param timeout_ms Timeout in milliseconds
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_TIMEOUT on timeout,
 *         HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_PollForConversion(ADC_HandleStruct *hadc, uint32_t timeout_ms) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_PollForConversion(&hadc->hal_handle, timeout_ms);
}

/**
 * @brief Read the latest conversion result
 *
 * Reads the DR (Data Register) via HAL_ADC_GetValue(). Does not start
 * a new conversion.
 *
 * @param hadc ADC handle
 * @param value Destination for the raw value (0..max for current resolution)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready or value is NULL
 */
HAL_StatusTypeDef ADC_GetValue(ADC_HandleStruct *hadc, uint32_t *value) {
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

/**
 * @brief Configure, convert and read one channel (convenience function)
 *
 * Combines ADC_ConfigChannel(), ADC_StartConversion(), ADC_PollForConversion(),
 * and ADC_GetValue() into a single call. Uses a default sampling time of
 * 84 cycles (good general-purpose value for 12-bit at moderate speeds).
 *
 * @param hadc ADC handle
 * @param channel Channel to read (ADC_CHANNEL_0..18)
 * @param value Destination for the raw value
 * @retval HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ReadChannel(ADC_HandleStruct *hadc, uint32_t channel, uint32_t *value) {
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

/**
 * @brief Start free-running continuous conversions
 *
 * Enables continuous conversion mode in the HAL handle and starts the ADC.
 * The ADC will continuously convert the configured channel(s), updating DR
 * on each completion. Use ADC_GetValue() to read the latest value anytime.
 *
 * @param hadc ADC handle (must be initialized, channel configured)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_StartContinuousConversion(ADC_HandleStruct *hadc) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    hadc->hal_handle.Init.ContinuousConvMode = ENABLE;

    return HAL_ADC_Start(&hadc->hal_handle);
}

/**
 * @brief Stop free-running continuous conversions
 *
 * Calls HAL_ADC_Stop() which disables the ADC and stops continuous conversion.
 *
 * @param hadc ADC handle
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_StopContinuousConversion(ADC_HandleStruct *hadc) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Stop(&hadc->hal_handle);
}

/**
 * @brief Stream conversions into a buffer via DMA
 *
 * Starts DMA transfer from ADC DR to the provided buffer. The ADC runs
 * in continuous mode; each conversion triggers a DMA request. The buffer
 * is filled with raw conversion values. Caller must ensure buffer size
 * matches @p length.
 *
 * @param hadc ADC handle (must be initialized with dma_enabled=true)
 * @param buffer Destination buffer (uint32_t array)
 * @param length Number of conversions to transfer
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready or DMA not configured
 */
HAL_StatusTypeDef ADC_StartDMA(ADC_HandleStruct *hadc, uint32_t *buffer, uint32_t length) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    if (buffer == NULL || length == 0) {
        return HAL_ERROR;
    }

    return HAL_ADC_Start_DMA(&hadc->hal_handle, buffer, length);
}

/**
 * @brief Stop a DMA stream
 *
 * Calls HAL_ADC_Stop_DMA() which stops the DMA transfer and the ADC.
 *
 * @param hadc ADC handle
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_StopDMA(ADC_HandleStruct *hadc) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Stop_DMA(&hadc->hal_handle);
}

/**
 * @brief Convert a scan sequence into a buffer and wait for it
 *
 * Performs a multi-channel scan conversion via DMA. The channel sequence
 * must be configured beforehand with ADC_ConfigMultiChannel(). Waits for
 * the DMA transfer to complete (all channels converted once) by polling
 * the conv_complete_flag set by the DMA callback in adc_events.c.
 *
 * @param hadc ADC handle (must be initialized with dma_enabled=true)
 * @param values Destination buffer (size >= num_channels)
 * @param num_channels Number of channels in the sequence
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready or DMA not enabled,
 *         HAL_TIMEOUT if conversion doesn't complete in time
 */
HAL_StatusTypeDef ADC_ReadMultiChannel(ADC_HandleStruct *hadc, uint32_t *values,
                                       uint32_t num_channels) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    if (!hadc->config.dma_enabled) {
        return HAL_ERROR;
    }

    hadc->conv_complete_flag = false;

    status = ADC_StartDMA(hadc, values, num_channels);
    if (status != HAL_OK) {
        return status;
    }

    /* The sequence finishes in the DMA interrupt, so wait on the flag it sets.
       Polling the ADC here would consume the status bits the DMA path needs. */
    const uint32_t start = HAL_GetTick();
    while (!hadc->conv_complete_flag) {
        if ((HAL_GetTick() - start) > ADC_CONVERSION_TIMEOUT) {
            (void)ADC_StopDMA(hadc);
            return HAL_TIMEOUT;
        }
    }

    return ADC_StopDMA(hadc);
}

/**
 * @brief Start an interrupt-driven conversion
 *
 * Clears the completion flag and calls HAL_ADC_Start_IT() which enables
 * the EOC interrupt and starts a single conversion. On completion,
 * HAL_ADC_ConvCpltCallback() is called, which routes to the registered
 * conv_complete_cb with the converted value.
 *
 * @param hadc ADC handle (must be initialized)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_Start_IT(ADC_HandleStruct *hadc) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    hadc->conv_complete_flag = false;

    return HAL_ADC_Start_IT(&hadc->hal_handle);
}

/**
 * @brief Stop an interrupt-driven conversion
 *
 * Calls HAL_ADC_Stop_IT() which disables the EOC interrupt and stops the ADC.
 *
 * @param hadc ADC handle
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_Stop_IT(ADC_HandleStruct *hadc) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_ADC_Stop_IT(&hadc->hal_handle);
}

/**
 * @brief Configure a channel then start an interrupt-driven conversion
 *
 * Convenience function: ADC_ConfigChannel() + ADC_Start_IT().
 * Uses default sampling time of 84 cycles.
 *
 * @param hadc ADC handle
 * @param channel Channel to read
 * @retval HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ReadChannel_IT(ADC_HandleStruct *hadc, uint32_t channel) {
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

/**
 * @brief Register the conversion complete callback
 *
 * Called from HAL_ADC_ConvCpltCallback() in ISR context. The callback
 * receives the handle and the raw converted value. Set to NULL to clear.
 *
 * @param hadc ADC handle
 * @param callback Function pointer, or NULL to clear
 */
void ADC_RegisterConvCompleteCallback(ADC_HandleStruct *hadc,
                                      void (*callback)(ADC_HandleStruct *, uint32_t)) {
    if (hadc != NULL) {
        hadc->conv_complete_cb = callback;
    }
}

/**
 * @brief Register the error callback
 *
 * Called from HAL_ADC_ErrorCallback() in ISR context on overrun, DMA error,
 * etc. Set to NULL to clear.
 *
 * @param hadc ADC handle
 * @param callback Function pointer, or NULL to clear
 */
void ADC_RegisterErrorCallback(ADC_HandleStruct *hadc, void (*callback)(ADC_HandleStruct *)) {
    if (hadc != NULL) {
        hadc->error_cb = callback;
    }
}
