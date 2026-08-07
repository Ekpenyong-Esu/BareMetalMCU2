/**
 * @file dac_types.h
 * @brief Data types and constants for the DAC driver
 */

#ifndef DAC_TYPES_H
#define DAC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dac.h"
#include <stdbool.h>
#include <stdint.h>

/** @brief Largest code the 12-bit converter accepts */
#define DAC_MAX_VALUE_12BIT     4095U

/** @brief Analog reference of the STM32F429I Discovery board, in volts */
#define DAC_REFERENCE_VOLTAGE   3.3f

/** @brief Pin carrying DAC_OUT1 on the Discovery board */
#define DAC_OUT1_PORT           GPIOA
#define DAC_OUT1_PIN            GPIO_PIN_4

/**
 * @brief DAC configuration
 */
typedef struct {
    uint32_t channel;        /**< Channel to configure; only DAC_CHANNEL_1 is wired */
    uint32_t trigger;        /**< DAC_TRIGGER_NONE, DAC_TRIGGER_SOFTWARE or a timer TRGO */
    uint32_t output_buffer;  /**< DAC_OUTPUTBUFFER_ENABLE or DAC_OUTPUTBUFFER_DISABLE */
} DAC_ConfigTypeDef;

/**
 * @brief DAC handle
 */
typedef struct DAC_HandleStruct {
    DAC_HandleTypeDef hal_handle;  /**< HAL DAC handle */
    DAC_ConfigTypeDef config;      /**< Configuration in force */
    bool initialized;              /**< Initialization status */
} DAC_HandleStruct;

#ifdef __cplusplus
}
#endif

#endif /* DAC_TYPES_H */
