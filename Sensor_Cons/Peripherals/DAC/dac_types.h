/**
 * @file    dac_types.h
 * @brief   Shared types for the DAC driver
 * @details This file holds the basic types and settings for DAC.
 *
 * How it works (in simple words):
 * - DAC does the opposite of ADC: it turns a number into a voltage.
 * - Give it 0 and you get 0V. Give it 4095 and you get about 3.3V.
 * - In between gives a voltage in between.
 * - The chip has two outputs; the application says which channel and
 *   which pin it uses (on STM32F4: channel 1 on PA4, channel 2 on PA5).
 * - You can start the voltage by software or by a timer.
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
#define DAC_MAX_VALUE_12BIT 4095U
#define DAC_MAX_VALUE_8BIT 255U

/** @brief Analog reference of the STM32F429I Discovery board, in volts */
#define DAC_REFERENCE_VOLTAGE 3.3f

/**
 * @brief Settings for the DAC
 * @details Pick which channel and which pin carries it, how to start
 *          (software or timer), if the output helper is on, and how
 *          the number is lined up (12-bit or 8-bit).
 */
typedef struct {
    uint32_t channel;       /**< DAC_CHANNEL_1 or DAC_CHANNEL_2 */
    GPIO_TypeDef *outPort;  /**< Port of the pin that carries this channel's output */
    uint16_t outPin;        /**< Pin that carries this channel's output (GPIO_PIN_x) */
    uint32_t trigger;       /**< How to start: software or timer */
    uint32_t output_buffer; /**< Helper that makes the output stronger (on/off) */
    uint32_t alignment;     /**< How the number is lined up (12-bit or 8-bit) */
} DAC_ConfigTypeDef;

/**
 * @brief   Biggest number allowed for the chosen mode
 * @param   alignment How the number is lined up (12-bit or 8-bit)
 * @retval  Biggest valid number (255 for 8-bit, 4095 for 12-bit)
 */
static inline uint32_t DAC_MaxValueFor(uint32_t alignment) {
    return (alignment == DAC_ALIGN_8B_R) ? DAC_MAX_VALUE_8BIT : DAC_MAX_VALUE_12BIT;
}

/**
 * @brief Handle that keeps all DAC info in one place
 * @details Holds the low-level handle, settings, and if it is ready.
 */
typedef struct DAC_HandleStruct {
    DAC_HandleTypeDef hal_handle; /**< Low-level DAC handle */
    DAC_ConfigTypeDef config;     /**< Settings in use */
    bool initialized;             /**< True if ready to use */
} DAC_HandleStruct;

#ifdef __cplusplus
}
#endif

#endif /* DAC_TYPES_H */
