/**
 * @file dac_output.c
 * @brief DAC conversion control and output value access
 */

#include "dac_output.h"
#include "dac_core.h"

HAL_StatusTypeDef DAC_SetValue(DAC_HandleStruct *hdac, uint32_t channel, uint32_t value)
{
    if (!DAC_IsChannelValid(hdac, channel) || (value > DAC_MAX_VALUE_12BIT)) {
        return HAL_ERROR;
    }

    if (HAL_DAC_SetValue(&hdac->hal_handle, channel, DAC_ALIGN_12B_R, value) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_DAC_Start(&hdac->hal_handle, channel);
}

HAL_StatusTypeDef DAC_Start(DAC_HandleStruct *hdac, uint32_t channel)
{
    if (!DAC_IsChannelValid(hdac, channel)) {
        return HAL_ERROR;
    }

    return HAL_DAC_Start(&hdac->hal_handle, channel);
}

HAL_StatusTypeDef DAC_Stop(DAC_HandleStruct *hdac, uint32_t channel)
{
    if (!DAC_IsChannelValid(hdac, channel)) {
        return HAL_ERROR;
    }

    return HAL_DAC_Stop(&hdac->hal_handle, channel);
}

uint32_t DAC_GetValue(const DAC_HandleStruct *hdac, uint32_t channel)
{
    if (!DAC_IsChannelValid(hdac, channel)) {
        return 0U;
    }

    return HAL_DAC_GetValue(&hdac->hal_handle, channel);
}
