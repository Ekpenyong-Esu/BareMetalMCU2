/**
 * @file    adc_core.c
 * @brief   ADC lifecycle and handle registry implementation
 *
 * This module implements the ADC core functionality:
 * - Initialization/deinitialization with clock, GPIO, DMA setup
 * - Channel configuration (single and multi-channel scan)
 * - Handle registry for HAL callback routing (instance -> handle)
 * - Runtime reconfiguration (resolution, sampling time)
 * - Status queries
 *
 * Key Design Points:
 * - Handle registry (s_handles) maps ADC instance index -> handle
 * - Registry populated in ADC_Init() before HAL_ADC_Init() so MspInit
 *   can find the handle for DMA/clock setup
 * - ADC_Register/Unregister are the only places touching s_handles
 * - ADC_ApplyChannel() handles GPIO config + HAL channel config
 * - Multi-channel scan: configures each channel at successive ranks,
 *   then re-inits HAL with ScanConvMode enabled
 */

#include "adc_core.h"

#include "adc_channels.h"
#include "adc_hw.h"
#include "log.h"

#include <stddef.h>

/* Private variables ---------------------------------------------------------*/

/** @brief Instance index to owning handle; the only file-scope state here */
static ADC_HandleStruct *s_handles[ADC_INSTANCE_COUNT] = {0};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Register a handle in the instance->handle map
 *
 * Called during ADC_Init() before HAL_ADC_Init() so that
 * HAL_ADC_MspInit() can find the handle for DMA configuration.
 *
 * @param hadc Handle to register
 */
static void ADC_Register(ADC_HandleStruct *hadc) {
    uint32_t index = ADC_InstanceIndex(hadc->hal_handle.Instance);
    if (index < ADC_INSTANCE_COUNT) {
        s_handles[index] = hadc;
    }
}

/**
 * @brief Unregister a handle from the instance->handle map
 *
 * Called during ADC_DeInit() and on init failure.
 *
 * @param hadc Handle to unregister
 */
static void ADC_Unregister(const ADC_HandleStruct *hadc) {
    uint32_t index = ADC_InstanceIndex(hadc->hal_handle.Instance);
    if (index < ADC_INSTANCE_COUNT && s_handles[index] == hadc) {
        s_handles[index] = NULL;
    }
}

/**
 * @brief Apply a single channel configuration (GPIO + HAL)
 *
 * Configures the GPIO pin for analog mode, then sets up the HAL
 * channel configuration with the given rank and sampling time.
 *
 * @param hadc ADC handle
 * @param channel ADC channel
 * @param sampling_time Sampling time
 * @param rank Rank in the conversion sequence (1-based)
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADC_ApplyChannel(ADC_HandleStruct *hadc, uint32_t channel,
                                          uint32_t sampling_time, uint32_t rank) {
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

/**
 * @brief Initialize an ADC handle
 *
 * Full initialization sequence:
 * 1. Validate args, select instance (default ADC1)
 * 2. Enable ADC instance clock (RCC)
 * 3. Configure GPIO pin for the initial channel (analog mode)
 * 4. Copy config, set default VDDA (3.3V nominal)
 * 5. Configure HAL ADC handle:
 *    - Clock prescaler: PCLK/4 (max ADC clock 36MHz on F4)
 *    - Resolution: from config (validated)
 *    - ScanConvMode: disabled initially (single channel)
 *    - ContinuousConvMode: from config
 *    - External trigger: software start
 *    - Data alignment: right
 *    - NbrOfConversion: 1
 *    - DMAContinuousRequests: from config
 *    - EOCSelection: single conversion
 * 6. Configure DMA if enabled
 * 7. Register handle in instance map (before HAL init for MspInit)
 * 8. HAL_ADC_Init()
 * 9. Mark initialized
 *
 * @param hadc Handle to initialize (must be zeroed)
 * @param config Configuration (instance, channel, resolution, sampling_time, conv_mode,
 * dma_enabled)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef ADC_Init(ADC_HandleStruct *hadc, const ADC_ConfigTypeDef *config) {
    if (hadc == NULL || config == NULL) {
        return HAL_ERROR;
    }

    log_debug("ADC: Initializing ADC");

    ADC_TypeDef *instance = (config->instance != NULL) ? config->instance : ADC1;
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
    hadc->vdda = ADC_VDDA_NOMINAL;

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

/**
 * @brief Release an ADC handle and unregister it
 *
 * Deinitialization sequence:
 * 1. Check handle is ready
 * 2. HAL_ADC_DeInit() - deinitializes peripheral, calls MspDeInit
 * 3. HAL_DMA_DeInit() if DMA was enabled
 * 4. Unregister from instance map
 * 5. Clear initialized flag
 *
 * @param hadc Handle to release
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef ADC_DeInit(ADC_HandleStruct *hadc) {
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

/**
 * @brief Configure a single channel as the conversion sequence
 *
 * Convenience wrapper: ADC_ApplyChannel() with rank 1.
 * Also updates the handle's config.channel.
 *
 * @param hadc ADC handle
 * @param channel Channel to convert
 * @param sampling_time Sampling time for that channel
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADC_ConfigChannel(ADC_HandleStruct *hadc, uint32_t channel,
                                    uint32_t sampling_time) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return ADC_ApplyChannel(hadc, channel, sampling_time, 1);
}

/**
 * @brief Configure a scan sequence of several channels
 *
 * Configures each channel at successive ranks (1..num_channels) with
 * individual sampling times. Then re-initializes the HAL with:
 * - ScanConvMode = ENABLE (if >1 channel)
 * - NbrOfConversion = num_channels
 *
 * @param hadc ADC handle
 * @param channels Array of channels in conversion order
 * @param sampling_times Array of sampling times (same length)
 * @param num_channels Number of entries in both arrays
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADC_ConfigMultiChannel(ADC_HandleStruct *hadc, const uint32_t *channels,
                                         const uint32_t *sampling_times, uint32_t num_channels) {
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

    return HAL_ADC_Init(&hadc->hal_handle);
}

/**
 * @brief Change the conversion resolution
 *
 * Updates the HAL handle's resolution and re-initializes the ADC.
 * Also updates the handle's config.resolution.
 *
 * @param hadc ADC handle
 * @param resolution New resolution (ADC_RESOLUTION_12B, 10B, 8B, 6B)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADC_SetResolution(ADC_HandleStruct *hadc, uint32_t resolution) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    hadc->hal_handle.Init.Resolution = ADC_ValidateResolution(resolution);
    hadc->config.resolution = resolution;

    return HAL_ADC_Init(&hadc->hal_handle);
}

/**
 * @brief Change the sampling time of a channel
 *
 * Reuses ADC_ConfigChannel() which re-applies GPIO + channel config.
 *
 * @param hadc ADC handle
 * @param channel Channel to adjust
 * @param sampling_time New sampling time
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADC_SetSamplingTime(ADC_HandleStruct *hadc, uint32_t channel,
                                      uint32_t sampling_time) {
    return ADC_ConfigChannel(hadc, channel, sampling_time);
}

/**
 * @brief Current driver status
 *
 * Returns HAL_OK if ready, HAL_BUSY if a conversion is ongoing,
 * HAL_ERROR if not initialized or in error state.
 *
 * @param hadc ADC handle
 * @retval HAL_StatusTypeDef HAL_OK, HAL_BUSY or HAL_ERROR
 */
HAL_StatusTypeDef ADC_GetStatus(const ADC_HandleStruct *hadc) {
    HAL_StatusTypeDef status = ADC_CheckReady(hadc);
    if (status != HAL_OK) {
        return status;
    }

    return (hadc->hal_handle.State == HAL_ADC_STATE_BUSY) ? HAL_BUSY : HAL_OK;
}

/**
 * @brief Whether the handle is initialized
 *
 * @param hadc ADC handle
 * @retval bool true when usable (initialized and not in error)
 */
bool ADC_IsReady(const ADC_HandleStruct *hadc) {
    return ADC_CheckReady(hadc) == HAL_OK;
}

/**
 * @brief Whether a conversion has finished
 *
 * Checks the EOC (End Of Conversion) flag in the ADC status register.
 *
 * @param hadc ADC handle
 * @retval bool true when the end-of-conversion flag is set
 */
bool ADC_IsConversionComplete(const ADC_HandleStruct *hadc) {
    if (ADC_CheckReady(hadc) != HAL_OK) {
        return false;
    }

    return (hadc->hal_handle.Instance->SR & ADC_FLAG_EOC) != 0U;
}

/**
 * @brief Mark a handle unusable after a fatal error
 *
 * Clears the initialized flag so all subsequent operations fail.
 * Called by ADC_ErrorHandler() in adc_events.c on overrun/DMA error.
 *
 * @param hadc ADC handle
 */
void ADC_ErrorHandler(ADC_HandleStruct *hadc) {
    if (hadc != NULL) {
        hadc->initialized = false;
    }
}

/**
 * @brief Get handle for a HAL ADC handle (used by callbacks)
 *
 * Looks up the handle in the instance registry. Called from
 * HAL_ADC_ConvCpltCallback() and HAL_ADC_ErrorCallback() in adc_events.c.
 *
 * @param hal HAL ADC handle
 * @retval ADC_HandleStruct* Matching handle, or NULL if not found
 */
ADC_HandleStruct *ADC_GetHandleFor(const ADC_HandleTypeDef *hal) {
    if (hal == NULL) {
        return NULL;
    }

    uint32_t index = ADC_InstanceIndex(hal->Instance);
    return (index < ADC_INSTANCE_COUNT) ? s_handles[index] : NULL;
}

/**
 * @brief Human-readable status string
 *
 * @param status HAL status code
 * @retval const char* Status string
 */
const char *ADC_GetStatusString(HAL_StatusTypeDef status) {
    switch (status) {
        case HAL_OK:
            return "OK";
        case HAL_ERROR:
            return "ERROR";
        case HAL_BUSY:
            return "BUSY";
        case HAL_TIMEOUT:
            return "TIMEOUT";
        default:
            return "UNKNOWN";
    }
}
