/**
 * @file    adc_types.h
 * @brief   Shared vocabulary for the ADC driver
 * @details Configuration, handle and status constants. Every other translation
 *          unit in the module depends on this header and nothing else.
 */

#ifndef ADC_TYPES_H
#define ADC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

#define ADC_MAX_VALUE_12BIT     4095U   /*!< Maximum value for 12-bit ADC */
#define ADC_MAX_VALUE_10BIT     1023U   /*!< Maximum value for 10-bit ADC */
#define ADC_MAX_VALUE_8BIT      255U    /*!< Maximum value for 8-bit ADC */
#define ADC_MAX_VALUE_6BIT      63U     /*!< Maximum value for 6-bit ADC */

#define ADC_REFERENCE_VOLTAGE   3.3f    /*!< Reference voltage in Volts */
#define ADC_TIMEOUT_MS          1000U   /*!< Default timeout in milliseconds */
#define ADC_CONVERSION_TIMEOUT  1000U   /*!< Conversion timeout in milliseconds */

#define ADC_INSTANCE_COUNT      3U      /*!< ADC1, ADC2, ADC3 */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief ADC conversion mode
 */
typedef enum {
    ADC_MODE_SINGLE     = 0,  /*!< Single conversion mode */
    ADC_MODE_CONTINUOUS = 1,  /*!< Continuous conversion mode */
} ADC_ConversionModeTypeDef;

/**
 * @brief ADC configuration
 */
typedef struct {
    ADC_TypeDef*                instance;       /*!< ADC1/ADC2/ADC3; NULL selects ADC1 */
    uint32_t                    channel;        /*!< ADC channel to configure */
    uint32_t                    resolution;     /*!< ADC resolution */
    uint32_t                    sampling_time;  /*!< Sampling time */
    ADC_ConversionModeTypeDef   conv_mode;      /*!< Conversion mode */
    bool                        dma_enabled;    /*!< Enable DMA for conversions */
} ADC_ConfigTypeDef;

/**
 * @brief ADC handle
 * @note  Self-contained: no file-scope state backs it, so several handles on
 *        different ADC instances may exist at once.
 */
typedef struct ADC_HandleStruct {
    ADC_HandleTypeDef   hal_handle;     /*!< STM32 HAL ADC handle */
    DMA_HandleTypeDef   hdma_adc;       /*!< DMA handle for ADC */
    ADC_ConfigTypeDef   config;         /*!< ADC configuration */
    bool                initialized;    /*!< Initialization status */
    void (*conv_complete_cb)(struct ADC_HandleStruct* hadc, uint32_t value);
    void (*error_cb)(struct ADC_HandleStruct* hadc);
    volatile uint32_t   last_value;         /*!< Last value captured in ISR context */
    volatile bool       conv_complete_flag; /*!< Set by the conversion complete ISR */
} ADC_HandleStruct;

/* Exported inline helpers ---------------------------------------------------*/

/**
 * @brief   Common entry guard shared by every public entry point
 * @param   hadc Handle to validate
 * @retval  HAL_OK when the handle is usable
 */
static inline HAL_StatusTypeDef ADC_CheckReady(const ADC_HandleStruct* hadc)
{
    if (hadc == NULL || !hadc->initialized) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* ADC_TYPES_H */
