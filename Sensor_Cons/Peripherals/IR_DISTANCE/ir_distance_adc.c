/**
  ******************************************************************************
  * @file    ir_distance_adc.c
  * @brief   ADC access for the IR distance sensor (internal)
  ******************************************************************************
  */

#include "ir_distance_adc.h"
#include "adc_core.h"
#include "adc_convert.h"
#include "log.h"

/* Sharp modules drive a slow, high-impedance analog output. */
#define IR_DISTANCE_ADC_SAMPLETIME  ADC_SAMPLETIME_56CYCLES

IR_DISTANCE_StatusTypeDef IR_DISTANCE_ADC_Init(ADC_HandleStruct *hadc, uint32_t channel)
{
    if (hadc == NULL) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    if (!ADC_IsReady(hadc)) {
        log_error("IR_DISTANCE: ADC handle is not initialized");
        return IR_DISTANCE_NOT_INITIALIZED;
    }

    if (ADC_ConfigChannel(hadc, channel, IR_DISTANCE_ADC_SAMPLETIME) != HAL_OK) {
        log_error("IR_DISTANCE: failed to configure ADC channel %lu", (unsigned long)channel);
        return IR_DISTANCE_ERROR;
    }

    return IR_DISTANCE_OK;
}

IR_DISTANCE_StatusTypeDef IR_DISTANCE_ADC_Read(ADC_HandleStruct *hadc, uint32_t channel,
                                               uint16_t *value)
{
    uint32_t raw;

    if (hadc == NULL || value == NULL) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    switch (ADC_ReadChannel(hadc, channel, &raw)) {
        case HAL_OK:
            *value = (uint16_t)raw;
            return IR_DISTANCE_OK;
        case HAL_TIMEOUT:
            return IR_DISTANCE_TIMEOUT;
        case HAL_BUSY:
            return IR_DISTANCE_BUSY;
        default:
            return IR_DISTANCE_ERROR;
    }
}
