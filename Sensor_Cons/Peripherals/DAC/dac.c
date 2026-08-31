/**
 * @file dac.c
 * @brief DAC initialization and lifecycle
 */

#include "dac_core.h"
#include "gpio.h"
#include "log.h"
#include <string.h>

/**
 * @brief   Validate a requested configuration
 * @note    The HAL only asserts these, and assertions are compiled out here.
 */
static bool DAC_ValidateConfig(const DAC_ConfigTypeDef *config)
{
    if (config == NULL) {
        return false;
    }

    if (config->channel != DAC_CHANNEL_1) {
        return false;
    }

    switch (config->trigger) {
        case DAC_TRIGGER_NONE:
        case DAC_TRIGGER_SOFTWARE:
        case DAC_TRIGGER_T2_TRGO:
        case DAC_TRIGGER_T4_TRGO:
        case DAC_TRIGGER_T5_TRGO:
        case DAC_TRIGGER_T6_TRGO:
        case DAC_TRIGGER_T7_TRGO:
        case DAC_TRIGGER_T8_TRGO:
        case DAC_TRIGGER_EXT_IT9:
            break;
        default:
            return false;
    }

    if ((config->output_buffer != DAC_OUTPUTBUFFER_ENABLE) &&
        (config->output_buffer != DAC_OUTPUTBUFFER_DISABLE)) {
        return false;
    }

    switch (config->alignment) {
        case DAC_ALIGN_12B_R:
        case DAC_ALIGN_12B_L:
        case DAC_ALIGN_8B_R:
            break;
        default:
            return false;
    }

    return true;
}

HAL_StatusTypeDef DAC_Init(DAC_HandleStruct *hdac, const DAC_ConfigTypeDef *config)
{
    DAC_ChannelConfTypeDef channelConfig = {0};

    if (hdac == NULL) {
        return HAL_ERROR;
    }

    memset(hdac, 0, sizeof(*hdac));

    if (!DAC_ValidateConfig(config)) {
        log_error("DAC: invalid configuration");
        return HAL_ERROR;
    }

    hdac->config = *config;
    hdac->hal_handle.Instance = DAC;

    if (HAL_DAC_Init(&hdac->hal_handle) != HAL_OK) {
        log_error("DAC: HAL_DAC_Init failed");
        return HAL_ERROR;
    }

    channelConfig.DAC_Trigger = config->trigger;
    channelConfig.DAC_OutputBuffer = config->output_buffer;

    if (HAL_DAC_ConfigChannel(&hdac->hal_handle, &channelConfig, config->channel) != HAL_OK) {
        (void)HAL_DAC_DeInit(&hdac->hal_handle);
        log_error("DAC: channel configuration failed");
        return HAL_ERROR;
    }

    hdac->initialized = true;
    log_debug("DAC: initialized");

    return HAL_OK;
}

HAL_StatusTypeDef DAC_DeInit(DAC_HandleStruct *hdac)
{
    if ((hdac == NULL) || !hdac->initialized) {
        return HAL_ERROR;
    }

    if (HAL_DAC_DeInit(&hdac->hal_handle) != HAL_OK) {
        return HAL_ERROR;
    }

    hdac->initialized = false;

    return HAL_OK;
}

bool DAC_IsInitialized(const DAC_HandleStruct *hdac)
{
    return ((hdac != NULL) && hdac->initialized);
}

bool DAC_IsChannelValid(const DAC_HandleStruct *hdac, uint32_t channel)
{
    return ((hdac != NULL) && hdac->initialized && (channel == hdac->config.channel));
}

/**
 * @brief   Enable the DAC clock and drive PA4 as an analog output
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
    GPIO_InitTypeDef gpioInit = {0};

    if (hdac->Instance != DAC) {
        return;
    }

    __HAL_RCC_DAC_CLK_ENABLE();

    /* The GPIO driver owns the port clock. */
    gpioInit.Pin = DAC_OUT1_PIN;
    gpioInit.Mode = GPIO_MODE_ANALOG;
    gpioInit.Pull = GPIO_NOPULL;
    GPIO_Driver_Pin_Init(DAC_OUT1_PORT, &gpioInit);
}

/**
 * @brief   Release the DAC clock and return PA4 to its reset state
 * @note    Without this the HAL weak default leaves the peripheral clocked and
 *          the pin in analog mode after DAC_DeInit().
 */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance != DAC) {
        return;
    }

    __HAL_RCC_DAC_CLK_DISABLE();
    HAL_GPIO_DeInit(DAC_OUT1_PORT, DAC_OUT1_PIN);
}
