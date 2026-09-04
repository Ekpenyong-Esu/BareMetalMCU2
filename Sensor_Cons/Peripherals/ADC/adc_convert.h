/**
 * @file    adc_convert.h
 * @brief   ADC conversion control: polling, interrupt and DMA
 * @details All three transfer styles for starting conversions and collecting
 *          results. Knows nothing about pins, clocks or voltage scaling.
 *
 * Conversion Modes:
 * 1. Polling (Single):
 *    - ADC_StartConversion() -> ADC_PollForConversion() -> ADC_GetValue()
 *    - Or use ADC_ReadChannel() for configure+convert+read in one call
 *    - Simple, blocking, good for occasional reads
 *
 * 2. Continuous (Free-running):
 *    - ADC_StartContinuousConversion() -> ADC_GetValue() anytime
 *    - ADC runs continuously, latest value always in DR register
 *    - Good for high-rate sampling without CPU overhead
 *
 * 3. DMA (Streaming):
 *    - ADC_StartDMA() fills a buffer automatically
 *    - Zero CPU overhead, best for high-rate continuous acquisition
 *    - ADC_ReadMultiChannel() for scan sequences
 *
 * 4. Interrupt:
 *    - ADC_Start_IT() / ADC_ReadChannel_IT()
 *    - Callback on conversion complete (ADC_RegisterConvCompleteCallback)
 *    - Good for event-driven architectures
 *
 * Multi-Channel Scan:
 * - Configure sequence with ADC_ConfigMultiChannel() (adc_core.h)
 * - Use ADC_ReadMultiChannel() for DMA scan, or continuous + polling
 */

#ifndef ADC_CONVERT_H
#define ADC_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/* Polling -------------------------------------------------------------------*/

/**
 * @brief   Start a software-triggered single conversion
 *
 * Triggers a conversion on the currently configured channel. The ADC must
 * be initialized and the channel configured beforehand.
 *
 * @param   hadc ADC handle (must be initialized)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready,
 *          HAL_BUSY if another conversion is ongoing
 */
HAL_StatusTypeDef ADC_StartConversion(ADC_HandleStruct *hadc);

/**
 * @brief   Block until the current conversion finishes
 *
 * Polls the EOC (End Of Conversion) flag until set or timeout expires.
 * Does not read the data register; use ADC_GetValue() afterwards.
 *
 * @param   hadc ADC handle
 * @param   timeout_ms Timeout in milliseconds
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_TIMEOUT on timeout,
 *          HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_PollForConversion(ADC_HandleStruct *hadc, uint32_t timeout_ms);

/**
 * @brief   Read the latest conversion result
 *
 * Reads the DR (Data Register) which holds the last completed conversion.
 * Does not start a new conversion.
 *
 * @param   hadc ADC handle
 * @param   value Destination for the raw value (0..max for current resolution)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_GetValue(ADC_HandleStruct *hadc, uint32_t *value);

/**
 * @brief   Configure, convert and read one channel (convenience function)
 *
 * Combines ADC_ConfigChannel(), ADC_StartConversion(), ADC_PollForConversion(),
 * and ADC_GetValue() into a single call. Useful for occasional single reads.
 *
 * @param   hadc ADC handle
 * @param   channel Channel to read (ADC_CHANNEL_0..18)
 * @param   value Destination for the raw value
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ReadChannel(ADC_HandleStruct *hadc, uint32_t channel, uint32_t *value);

/* Continuous ----------------------------------------------------------------*/

/**
 * @brief   Start free-running continuous conversions
 *
 * Enables continuous conversion mode and starts the ADC. The ADC will
 * continuously convert the configured channel(s), updating DR on each
 * completion. Use ADC_GetValue() to read the latest value anytime.
 *
 * @param   hadc ADC handle (must be initialized, channel configured)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_StartContinuousConversion(ADC_HandleStruct *hadc);

/**
 * @brief   Stop free-running continuous conversions
 *
 * Disables continuous conversion mode and stops the ADC.
 *
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_StopContinuousConversion(ADC_HandleStruct *hadc);

/* DMA -----------------------------------------------------------------------*/

/**
 * @brief   Stream conversions into a buffer via DMA
 *
 * Starts DMA transfer from ADC DR to the provided buffer. The ADC runs
 * in continuous mode; each conversion triggers a DMA request. The buffer
 * is filled with raw conversion values. Caller must ensure buffer size
 * matches @p length.
 *
 * @param   hadc ADC handle (must be initialized with dma_enabled=true)
 * @param   buffer Destination buffer (uint32_t array)
 * @param   length Number of conversions to transfer
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready or DMA not configured
 */
HAL_StatusTypeDef ADC_StartDMA(ADC_HandleStruct *hadc, uint32_t *buffer, uint32_t length);

/**
 * @brief   Stop a DMA stream
 *
 * Stops the DMA transfer and disables continuous conversion mode.
 *
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_StopDMA(ADC_HandleStruct *hadc);

/**
 * @brief   Convert a scan sequence into a buffer and wait for it
 *
 * Performs a multi-channel scan conversion via DMA. The channel sequence
 * must be configured beforehand with ADC_ConfigMultiChannel(). Waits for
 * the DMA transfer to complete (all channels converted once).
 *
 * @param   hadc ADC handle (must be initialized with dma_enabled=true)
 * @param   values Destination buffer (size >= num_channels)
 * @param   num_channels Number of channels in the sequence
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_ReadMultiChannel(ADC_HandleStruct *hadc, uint32_t *values,
                                       uint32_t num_channels);

/* Interrupt -----------------------------------------------------------------*/

/**
 * @brief   Start an interrupt-driven conversion
 *
 * Enables EOC interrupt and starts a single conversion. On completion,
 * HAL_ADC_ConvCpltCallback() is called, which routes to the registered
 * conv_complete_cb with the converted value.
 *
 * @param   hadc ADC handle (must be initialized)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_Start_IT(ADC_HandleStruct *hadc);

/**
 * @brief   Stop an interrupt-driven conversion
 *
 * Disables EOC interrupt and stops the ADC.
 *
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_Stop_IT(ADC_HandleStruct *hadc);

/**
 * @brief   Configure a channel then start an interrupt-driven conversion
 *
 * Convenience function: ADC_ConfigChannel() + ADC_Start_IT().
 *
 * @param   hadc ADC handle
 * @param   channel Channel to read
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ReadChannel_IT(ADC_HandleStruct *hadc, uint32_t channel);

/**
 * @brief   Register the conversion complete callback
 *
 * Called from HAL_ADC_ConvCpltCallback() in ISR context. The callback
 * receives the handle and the raw converted value. Set to NULL to clear.
 *
 * @param   hadc ADC handle
 * @param   callback Function pointer, or NULL to clear
 */
void ADC_RegisterConvCompleteCallback(ADC_HandleStruct *hadc,
                                      void (*callback)(ADC_HandleStruct *, uint32_t));

/**
 * @brief   Register the error callback
 *
 * Called from HAL_ADC_ErrorCallback() in ISR context on overrun, DMA error,
 * etc. Set to NULL to clear.
 *
 * @param   hadc ADC handle
 * @param   callback Function pointer, or NULL to clear
 */
void ADC_RegisterErrorCallback(ADC_HandleStruct *hadc, void (*callback)(ADC_HandleStruct *));

#ifdef __cplusplus
}
#endif

#endif /* ADC_CONVERT_H */
