/**
 * @file    adc_core.c
 * @brief   ADC lifecycle and handle registry implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "adc_core.h"

#include "adc_channels.h"
#include "adc_hw.h"
#include "log.h"

#include <stddef.h>

/* Private variables ---------------------------------------------------------*/

/** @brief Instance index to owning handle; the only file-scope state here */
static ADC_HandleStruct* s_handles[ADC_INSTANCE_COUNT] = {0};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Add a handle to the instance registry
 * @param   hadc Handle to register
 * @note    Lets HAL callbacks (which receive only the HAL handle) find the
 *          owning driver handle.
 */
static void ADC_Register(ADC_HandleStruct* hadc)
{
    uint32_t index = ADC_InstanceIndex(hadc->hal_handle.Instance);
    if (index < ADC_INSTANCE_COUNT) {
        s_handles[index] = hadc;
    }
}

/**
 * @brief   Remove a handle from the instance registry
 * @param   hadc Handle to remove; ignored if it is not the registered one
 */
static void ADC_Unregister(const ADC_HandleStruct* hadc)
{
    uint32_t index = ADC_InstanceIndex(hadc->hal_handle.Instance);
    if (index < ADC_INSTANCE_COUNT && s_handles[index] == hadc) {
        s_handles[index] = NULL;
    }
}

/**
 * @brief   Configure one channel: analog GPIO, rank and sampling time
 * @param   hadc ADC handle
 * @param   channel Channel to configure
 * @param   sampling_time Sampling time for that channel
 * @param   rank Position in the conversion sequence (1-based)
 * @retval  HAL_StatusTypeDef Status of the operation
 */
static HAL_StatusTypeDef ADC_ApplyChannel(ADC_HandleStruct* hadc, uint32_t channel,
                                          uint32_t sampling_time, uint32_t rank)
{
    HAL_StatusTypeDef status = ADC_ConfigureChannelGpio(channel);
    if (status != HAL_OK) {
        return status;
    }

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = rank;
    sConfig.SamplingTime = ADC_ValidateSamplingTime(sampling_time);

    return HAL_ADC_ConfigChannel(&hadc->hal_handle, &sConfig);
}

/* Exported functions --------------------------------------------------------*/

/* Brings up the peripheral, GPIO and (optionally) DMA for one handle. */
HAL_StatusTypeDef ADC_Init(ADC_HandleStruct* hadc, const ADC_ConfigTypeDef* config)
{
    if (hadc == NULL || config == NULL) {
        return HAL_ERROR;
    }

    log_debug("ADC: Initializing ADC");

    /* NULL instance defaults to ADC1. */
    ADC_TypeDef* instance = (config->instance != NULL) ? config->instance : ADC1;
    if (ADC_InstanceIndex(instance) >= ADC_INSTANCE_COUNT) {
        return HAL_ERROR;
    }

    if (ADC_EnableInstanceClock(instance) != HAL_OK) {
        return HAL_ERROR;
    }

    if (ADC_ConfigureChannelGpio(config->channel) != HAL_OK) {
        return HAL_ERROR;
    }

    hadc->config = *config;
    hadc->config.instance = instance;

    /* Single-channel, software-triggered setup: no scan, no external trigger. */
    hadc->hal_handle.Instance = instance;
    hadc->hal_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc->hal_handle.Init.Resolution = ADC_ValidateResolution(config->resolution);
    hadc->hal_handle.Init.ScanConvMode = DISABLE;
    hadc->hal_handle.Init.ContinuousConvMode =
        (config->conv_mode == ADC_MODE_CONTINUOUS) ? ENABLE : DISABLE;
    hadc->hal_handle.Init.DiscontinuousConvMode = DISABLE;
    hadc->hal_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc->hal_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc->hal_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc->hal_handle.Init.NbrOfConversion = 1;
    hadc->hal_handle.Init.DMAContinuousRequests = config->dma_enabled ? ENABLE : DISABLE;
    hadc->hal_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    if (config->dma_enabled && ADC_ConfigureDma(hadc) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Publish before HAL_ADC_Init so MspInit can resolve the handle */
    ADC_Register(hadc);

    HAL_StatusTypeDef status = HAL_ADC_Init(&hadc->hal_handle);
    if (status != HAL_OK) {
        ADC_Unregister(hadc);
        return status;
    }

    hadc->initialized = true;

    log_debug("ADC: ADC initialized successfully");

    return HAL_OK;
}

/* Tears down HAL, DMA and the registry entry in that order. */
HAL_StatusTypeDef ADC_DeInit(ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    status = HAL_ADC_DeInit(&hadc->hal_handle);
    if (status != HAL_OK) {
        return status;
    }

    if (hadc->config.dma_enabled) {
        status = HAL_DMA_DeInit(&hadc->hdma_adc);
        if (status != HAL_OK) {
            return status;
        }
    }

    ADC_Unregister(hadc);
    hadc->initialized = false;

    return HAL_OK;
}

/* Single-channel convenience wrapper around ADC_ApplyChannel(). */
HAL_StatusTypeDef ADC_ConfigChannel(ADC_HandleStruct* hadc, uint32_t channel,
                                    uint32_t sampling_time)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return ADC_ApplyChannel(hadc, channel, sampling_time, 1);
}

/* Applies every channel in order, then re-inits with the new sequence length. */
HAL_StatusTypeDef ADC_ConfigMultiChannel(ADC_HandleStruct* hadc,
                                         const uint32_t* channels,
                                         const uint32_t* sampling_times,
                                         uint32_t num_channels)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    if (channels == NULL || sampling_times == NULL || num_channels == 0) {
        return HAL_ERROR;
    }

    for (uint32_t i = 0; i < num_channels; i++) {
        status = ADC_ApplyChannel(hadc, channels[i], sampling_times[i], i + 1);
        if (status != HAL_OK) {
            return status;
        }
    }

    hadc->hal_handle.Init.NbrOfConversion = num_channels;
    hadc->hal_handle.Init.ScanConvMode = (num_channels > 1) ? ENABLE : DISABLE;

    /* Re-init so the new sequence length takes effect. */
    return HAL_ADC_Init(&hadc->hal_handle);
}

HAL_StatusTypeDef ADC_SetResolution(ADC_HandleStruct* hadc, uint32_t resolution)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    hadc->hal_handle.Init.Resolution = ADC_ValidateResolution(resolution);
    hadc->config.resolution = resolution;

    /* Re-init so the new resolution takes effect. */
    return HAL_ADC_Init(&hadc->hal_handle);
}

HAL_StatusTypeDef ADC_SetSamplingTime(ADC_HandleStruct* hadc, uint32_t channel,
                                      uint32_t sampling_time)
{
    return ADC_ConfigChannel(hadc, channel, sampling_time);
}

/* Reports HAL_BUSY mid-conversion instead of surfacing the raw HAL state. */
HAL_StatusTypeDef ADC_GetStatus(const ADC_HandleStruct* hadc)
{
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return (hadc->hal_handle.State == HAL_ADC_STATE_BUSY) ? HAL_BUSY : HAL_OK;
}

/* Thin wrapper so callers don't need to know about ADC_CheckReady(). */
bool ADC_IsReady(const ADC_HandleStruct* hadc)
{
    return ADC_CheckReady(hadc) == HAL_OK;
}

bool ADC_IsConversionComplete(const ADC_HandleStruct* hadc)
{
    if (ADC_CheckReady(hadc) != HAL_OK) {
        return false;
    }

    /* End-of-conversion flag in the status register. */
    return (hadc->hal_handle.Instance->SR & ADC_FLAG_EOC) != 0U;
}

/* A fatal error leaves the handle unusable until the next ADC_Init. */
void ADC_ErrorHandler(ADC_HandleStruct* hadc)
{
    if (hadc != NULL) {
        hadc->initialized = false;
    }
}

/* HAL callbacks carry only the HAL handle; the registry finds our handle. */
ADC_HandleStruct* ADC_GetHandleFor(const ADC_HandleTypeDef* hal)
{
    if (hal == NULL) {
        return NULL;
    }

    uint32_t index = ADC_InstanceIndex(hal->Instance);
    return (index < ADC_INSTANCE_COUNT) ? s_handles[index] : NULL;
}

/* The stream vector knows its ADC at compile time, so this only has to find
 * the handle that owns the matching DMA sub-handle. */
void ADC_DmaIrqHandler(ADC_TypeDef* instance)
{
    uint32_t index = ADC_InstanceIndex(instance);
    if (index < ADC_INSTANCE_COUNT && s_handles[index] != NULL) {
        HAL_DMA_IRQHandler(&s_handles[index]->hdma_adc);
    }
}

/* For log/debug output; not used for control flow. */
const char* ADC_GetStatusString(HAL_StatusTypeDef status)
{
    switch (status) {
        case HAL_OK:      return "OK";
        case HAL_ERROR:   return "ERROR";
        case HAL_BUSY:    return "BUSY";
        case HAL_TIMEOUT: return "TIMEOUT";
        default:          return "UNKNOWN";
    }
}
