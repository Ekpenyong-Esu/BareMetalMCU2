/**
 * @file dac_output.c
 * @brief DAC conversion control and output value access implementation
 *
 * This module implements the DAC output control functions:
 * - DAC_SetValue(): Loads code into DHR and starts conversion (software trigger)
 * - DAC_Start(): Starts conversion with already-loaded value (timer trigger)
 * - DAC_Stop(): Stops conversion, output holds last value
 * - DAC_GetValue(): Reads the current DOR (Data Output Register)
 *
 * All functions validate handle initialization and channel match (must be CH1).
 * DAC_SetValue also clamps the value to the maximum for the configured alignment.
 */

#include "dac_output.h"
#include "dac_core.h"

/**
 * @brief Load an output code and start the conversion (software trigger)
 *
 * Writes the value to the appropriate Data Holding Register (DHR) based on
 * the configured alignment, then triggers a software conversion if the
 * trigger is SOFTWARE/NONE. For timer triggers, this only loads the value;
 * the timer TRGO will start the actual conversion.
 *
 * Validates handle initialization and channel match (must be CH1).
 * Clamps value to the maximum for the configured alignment.
 *
 * @param hdac Handle (must be initialized)
 * @param channel Channel, must match the initialized channel (DAC_CHANNEL_1)
 * @param value Code from 0 to DAC_MAX_VALUE_12BIT (or 255 for 8-bit alignment)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid args or HAL failure
 */
HAL_StatusTypeDef DAC_SetValue(DAC_HandleStruct *hdac, uint32_t channel, uint32_t value)
{
    if (!DAC_IsChannelValid(hdac, channel) ||
        (value > DAC_MaxValueFor(hdac->config.alignment))) {
        return HAL_ERROR;
    }

    if (HAL_DAC_SetValue(&hdac->hal_handle, channel, hdac->config.alignment, value) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_DAC_Start(&hdac->hal_handle, channel);
}

/**
 * @brief Start the conversion using the code already loaded
 *
 * For timer-triggered mode: triggers a software conversion (SWTRIG) to
 * immediately convert the currently loaded value. For software trigger
 * mode, this is a no-op (DAC_SetValue already started it).
 *
 * @param hdac Handle (must be initialized)
 * @param channel Channel, must match the initialized channel
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid args
 */
HAL_StatusTypeDef DAC_Start(DAC_HandleStruct *hdac, uint32_t channel)
{
    if (!DAC_IsChannelValid(hdac, channel)) {
        return HAL_ERROR;
    }

    return HAL_DAC_Start(&hdac->hal_handle, channel);
}

/**
 * @brief Stop the conversion, leaving the last value on the output
 *
 * Disables the DAC channel. The output pin holds the last converted value
 * (if output buffer enabled) or goes high-Z (if buffer disabled).
 *
 * @param hdac Handle (must be initialized)
 * @param channel Channel, must match the initialized channel
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid args
 */
HAL_StatusTypeDef DAC_Stop(DAC_HandleStruct *hdac, uint32_t channel)
{
    if (!DAC_IsChannelValid(hdac, channel)) {
        return HAL_ERROR;
    }

    return HAL_DAC_Stop(&hdac->hal_handle, channel);
}

/**
 * @brief Read the code currently held in the data output register
 *
 * Reads the DOR (Data Output Register) which holds the last converted value.
 * This is the actual analog output code, not the holding register.
 *
 * @param hdac Handle (must be initialized)
 * @param channel Channel, must match the initialized channel
 * @retval uint32_t Current code (0..4095), or 0 when the arguments are rejected
 */
uint32_t DAC_GetValue(const DAC_HandleStruct *hdac, uint32_t channel)
{
    if (!DAC_IsChannelValid(hdac, channel)) {
        return 0U;
    }

    return HAL_DAC_GetValue(&hdac->hal_handle, channel);
}
