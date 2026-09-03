/**
 * @file    adc_types.h
 * @brief   Shared types for the ADC driver
 * @details This file holds the basic types and settings for ADC.
 *          Other ADC files use what is defined here.
 *
 * How it works (in simple words):
 * - ADC turns a real voltage (from a sensor or pin) into a number.
 * - More bits means more detail. 12-bit = 0 to 4095, 10-bit = 0 to 1023,
 *   8-bit = 0 to 255, 6-bit = 0 to 63.
 * - To get volts: volts = number / max_number * supply_voltage.
 * - You can use ADC1, ADC2 and ADC3 at the same time. Each has its own handle.
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

#define ADC_VDDA_NOMINAL        3.3f    /*!< Assumed supply until ADC_CalibrateVdda() measures it */
#define ADC_VREFINT_TYPICAL     1.21f   /*!< Datasheet VREFINT: 1.18 min, 1.21 typ, 1.24 max */
#define ADC_TIMEOUT_MS          1000U   /*!< Default timeout in milliseconds */
#define ADC_CONVERSION_TIMEOUT  1000U   /*!< Conversion timeout in milliseconds */

#define ADC_INSTANCE_COUNT      3U      /*!< ADC1, ADC2, ADC3 */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief How the ADC takes readings
 * @details SINGLE = take one reading and stop.
 *          CONTINUOUS = keep taking readings over and over.
 */
typedef enum {
    ADC_MODE_SINGLE     = 0,  /*!< Take one reading then stop */
    ADC_MODE_CONTINUOUS = 1,  /*!< Keep reading again and again */
} ADC_ConversionModeTypeDef;

/**
 * @brief Settings for one ADC
 * @details Pick which ADC (ADC1/ADC2/ADC3), which pin/channel,
 *          how detailed the reading is (resolution), how long to
 *          sample, and if it should read once or keep reading.
 *          dma_enabled = true lets the ADC send data by itself.
 */
typedef struct {
    ADC_TypeDef*                instance;       /*!< Which ADC to use (ADC1/ADC2/ADC3, NULL = ADC1) */
    uint32_t                    channel;        /*!< Which pin/channel to read */
    uint32_t                    resolution;     /*!< How detailed the number is (12/10/8/6 bit) */
    uint32_t                    sampling_time;  /*!< How long to sample the voltage */
    ADC_ConversionModeTypeDef   conv_mode;      /*!< Read once or keep reading */
    bool                        dma_enabled;    /*!< True = move data without CPU help */
} ADC_ConfigTypeDef;

/**
 * @brief Handle that keeps all ADC info in one place
 * @details Holds the settings, status, and last reading.
 *          You can have more than one handle (one per ADC).
 *          The driver keeps a small list so it can find the
 *          right handle when a reading finishes.
 */
typedef struct ADC_HandleStruct {
    ADC_HandleTypeDef   hal_handle;     /*!< Low-level ADC handle */
    DMA_HandleTypeDef   hdma_adc;       /*!< Helper for moving data if DMA is on */
    ADC_ConfigTypeDef   config;         /*!< Settings for this ADC */
    bool                initialized;    /*!< True if ready to use */
    void (*conv_complete_cb)(struct ADC_HandleStruct* hadc, uint32_t value); /*!< Called when a reading is done */
    void (*error_cb)(struct ADC_HandleStruct* hadc);                         /*!< Called if something goes wrong */
    volatile uint32_t   last_value;         /*!< Last number read */
    volatile bool       conv_complete_flag; /*!< True when a new reading is ready */
    float               vdda;               /*!< Supply voltage used to turn numbers into volts */
} ADC_HandleStruct;

/* Exported inline helpers ---------------------------------------------------*/

/**
 * @brief   Check if the ADC handle is ready to use
 * @param   hadc Handle to check
 * @retval  HAL_OK if ready, HAL_ERROR if not
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
