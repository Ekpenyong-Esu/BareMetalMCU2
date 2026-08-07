/**
 * @file    adc_events.c
 * @brief   HAL callback bridge for the ADC driver
 * @details The HAL calls these with a bare HAL handle; they resolve the owning
 *          driver handle through the registry and forward to the user callback.
 *          Deliberately has no header: nothing may call these directly.
 */

/* Includes ------------------------------------------------------------------*/
#include "adc_core.h"

#include <stddef.h>

/* HAL callbacks -------------------------------------------------------------*/

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    ADC_HandleStruct* handle = ADC_GetHandleFor(hadc);
    if (handle == NULL) {
        return;
    }

    uint32_t value = HAL_ADC_GetValue(hadc);
    handle->last_value = value;
    handle->conv_complete_flag = true;

    if (handle->conv_complete_cb != NULL) {
        handle->conv_complete_cb(handle, value);
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
    ADC_HandleStruct* handle = ADC_GetHandleFor(hadc);
    if (handle == NULL) {
        return;
    }

    if (handle->error_cb != NULL) {
        handle->error_cb(handle);
    }
}
