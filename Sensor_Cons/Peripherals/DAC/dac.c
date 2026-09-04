/**
 * @file dac.c
 * @brief DAC initialization and lifecycle implementation
 *
 * This module implements the DAC core functionality:
 * - Configuration validation (channel, output pin, trigger, buffer, alignment)
 * - HAL DAC initialization with channel configuration
 * - Output pin and clock management
 * - Handle state tracking
 *
 * Key Design Points:
 * - The application chooses the channel and the pin that carries it; the
 *   driver assumes no board wiring
 * - Configuration validated before HAL init to catch errors early
 * - The output pin is configured directly in DAC_Init() from the config, so
 *   the MSP only has to gate the clock and needs no handle lookup
 * - On DeInit, the DAC clock is disabled and the pin returned to reset state
 */

#include "dac_core.h"
#include "gpio.h"
#include "log.h"
#include <string.h>

/**
 * @brief   Validate a requested configuration
 *
 * The HAL only asserts these internally, and assertions are compiled out
 * in release builds. This explicit validation catches errors early.
 *
 * Validates:
 * - config not NULL
 * - channel == DAC_CHANNEL_1 or DAC_CHANNEL_2
 * - outPort/outPin given
 * - trigger: NONE, SOFTWARE, or valid timer TRGO (TIM2,4,5,6,7,8) or EXTI9
 * - output_buffer: ENABLE or DISABLE
 * - alignment: 12B_R, 12B_L, or 8B_R
 *
 * @param config Configuration to validate
 * @retval true if valid, false otherwise
 */
static bool DAC_ValidateConfig(const DAC_ConfigTypeDef *config) {
    if (config == NULL) {
        return false;
    }

    if ((config->channel != DAC_CHANNEL_1) && (config->channel != DAC_CHANNEL_2)) {
        return false;
    }

    if ((config->outPort == NULL) || (config->outPin == 0U)) {
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

/**
 * @brief Initialize the DAC peripheral
 *
 * Initialization sequence:
 * 1. Zero the handle
 * 2. Validate configuration (channel, pin, trigger, buffer, alignment)
 * 3. Store config in handle
 * 4. Drive the output pin as analog, so the digital stage never fights the DAC
 * 5. HAL_DAC_Init() - initializes DAC peripheral, calls MspInit (clock)
 * 6. HAL_DAC_ConfigChannel() - configures trigger and output buffer
 * 7. Mark handle as initialized
 *
 * On failure at any step, rolls back (pin released, DeInit if HAL init succeeded).
 *
 * @param hdac Handle to populate (must be zeroed or uninitialized)
 * @param config Requested configuration
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid config or HAL failure
 */
HAL_StatusTypeDef DAC_Init(DAC_HandleStruct *hdac, const DAC_ConfigTypeDef *config) {
    DAC_ChannelConfTypeDef channelConfig = {0};
    GPIO_InitTypeDef gpioInit = {0};

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

    /* The GPIO driver owns the port clock. */
    gpioInit.Pin = config->outPin;
    gpioInit.Mode = GPIO_MODE_ANALOG;
    gpioInit.Pull = GPIO_NOPULL;
    if (GPIO_Driver_Pin_Init(config->outPort, &gpioInit) != HAL_OK) {
        log_error("DAC: output pin init failed");
        return HAL_ERROR;
    }

    if (HAL_DAC_Init(&hdac->hal_handle) != HAL_OK) {
        (void)GPIO_Driver_Pin_DeInit(config->outPort, config->outPin);
        log_error("DAC: HAL_DAC_Init failed");
        return HAL_ERROR;
    }

    channelConfig.DAC_Trigger = config->trigger;
    channelConfig.DAC_OutputBuffer = config->output_buffer;

    if (HAL_DAC_ConfigChannel(&hdac->hal_handle, &channelConfig, config->channel) != HAL_OK) {
        (void)HAL_DAC_DeInit(&hdac->hal_handle);
        (void)GPIO_Driver_Pin_DeInit(config->outPort, config->outPin);
        log_error("DAC: channel configuration failed");
        return HAL_ERROR;
    }

    hdac->initialized = true;
    log_debug("DAC: initialized");

    return HAL_OK;
}

/**
 * @brief Deinitialize the DAC peripheral and release its clock and pin
 *
 * Calls HAL_DAC_DeInit() which invokes HAL_DAC_MspDeInit() to disable
 * the DAC clock, then returns the output pin to its reset state.
 * Clears initialized flag.
 *
 * @param hdac Handle
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not initialized or HAL fails
 */
HAL_StatusTypeDef DAC_DeInit(DAC_HandleStruct *hdac) {
    if ((hdac == NULL) || !hdac->initialized) {
        return HAL_ERROR;
    }

    if (HAL_DAC_DeInit(&hdac->hal_handle) != HAL_OK) {
        return HAL_ERROR;
    }

    (void)GPIO_Driver_Pin_DeInit(hdac->config.outPort, hdac->config.outPin);

    hdac->initialized = false;

    return HAL_OK;
}

/**
 * @brief Report whether the handle has been initialized
 *
 * @param hdac Handle
 * @retval bool true when initialized
 */
bool DAC_IsInitialized(const DAC_HandleStruct *hdac) {
    return ((hdac != NULL) && hdac->initialized);
}

/**
 * @brief Report whether a channel argument matches the initialized channel
 *
 * The HAL treats every value other than DAC_CHANNEL_1 as channel 2,
 * so an unchecked argument silently drives a pin this handle never
 * configured. This guard prevents that.
 *
 * @param hdac Handle
 * @param channel Channel supplied by the caller
 * @retval bool true when the channel may be used with this handle
 */
bool DAC_IsChannelValid(const DAC_HandleStruct *hdac, uint32_t channel) {
    return ((hdac != NULL) && hdac->initialized && (channel == hdac->config.channel));
}

/**
 * @brief Enable the DAC clock
 *
 * Called by HAL_DAC_Init() via the MSP mechanism. The output pin is
 * configured by DAC_Init() itself, from the config, so nothing here
 * depends on board wiring.
 *
 * @param hdac HAL DAC handle
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance != DAC) {
        return;
    }

    __HAL_RCC_DAC_CLK_ENABLE();
}

/**
 * @brief Release the DAC clock
 *
 * Called by HAL_DAC_DeInit() via the MSP mechanism. Without this, the HAL
 * weak default leaves the peripheral clocked after DAC_DeInit(). The pin
 * is released by DAC_DeInit(), which knows which one it was.
 *
 * @param hdac HAL DAC handle
 */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac) {
    if (hdac->Instance != DAC) {
        return;
    }

    __HAL_RCC_DAC_CLK_DISABLE();
}
