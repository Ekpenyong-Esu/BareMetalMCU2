/**
 ******************************************************************************
 * @file    ir_distance_types.h
 * @brief   Shared vocabulary for the IR distance sensor driver
 * @details Status codes, sensor types, configuration, calibration curve and
 *          handle. Every other translation unit depends on this header and
 *          nothing else.
 ******************************************************************************
 */

#ifndef IR_DISTANCE_TYPES_H
#define IR_DISTANCE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "adc_types.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

#define IR_DISTANCE_MAX_CURVE_POINTS 10U
#define IR_DISTANCE_MIN_CURVE_POINTS 2U

#define IR_DISTANCE_DEFAULT_AVERAGING_SAMPLES 5U
#define IR_DISTANCE_MAX_AVERAGING_SAMPLES 100U
#define IR_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT 100U /* ms */
#define IR_DISTANCE_SAMPLE_INTERVAL_MS 1U

/* Sensor specific ranges (mm) */
#define IR_DISTANCE_GP2Y0A21YK_MIN 100U  /* 10cm */
#define IR_DISTANCE_GP2Y0A21YK_MAX 800U  /* 80cm */
#define IR_DISTANCE_GP2Y0A02YK_MIN 200U  /* 20cm */
#define IR_DISTANCE_GP2Y0A02YK_MAX 1500U /* 150cm */
#define IR_DISTANCE_GP2Y0A41SK_MIN 40U   /* 4cm */
#define IR_DISTANCE_GP2Y0A41SK_MAX 300U  /* 30cm */
#define IR_DISTANCE_GP2Y0A51SK_MIN 20U   /* 2cm */
#define IR_DISTANCE_GP2Y0A51SK_MAX 150U  /* 15cm */

/* Range used when no predefined curve applies */
#define IR_DISTANCE_CUSTOM_MIN 10U
#define IR_DISTANCE_CUSTOM_MAX 1000U

/* Exported types ------------------------------------------------------------*/

/**
 * @brief IR distance sensor status enumeration
 */
typedef enum {
    IR_DISTANCE_OK = 0,          /**< Operation completed successfully */
    IR_DISTANCE_ERROR,           /**< General error occurred */
    IR_DISTANCE_BUSY,            /**< Sensor is busy measuring */
    IR_DISTANCE_TIMEOUT,         /**< Measurement timeout */
    IR_DISTANCE_INVALID_PARAM,   /**< Invalid parameter provided */
    IR_DISTANCE_NOT_INITIALIZED, /**< Driver not initialized */
    IR_DISTANCE_OUT_OF_RANGE     /**< Distance out of sensor range */
} IR_DISTANCE_StatusTypeDef;

/**
 * @brief IR distance sensor type enumeration
 */
typedef enum {
    IR_DISTANCE_GP2Y0A21YK = 0, /**< Sharp GP2Y0A21YK (10-80cm) */
    IR_DISTANCE_GP2Y0A02YK,     /**< Sharp GP2Y0A02YK (20-150cm) */
    IR_DISTANCE_GP2Y0A41SK,     /**< Sharp GP2Y0A41SK (4-30cm) */
    IR_DISTANCE_GP2Y0A51SK,     /**< Sharp GP2Y0A51SK (2-15cm) */
    IR_DISTANCE_CUSTOM          /**< Custom sensor with user-defined curve */
} IR_DISTANCE_SensorType_t;

/**
 * @brief IR distance sensor configuration structure
 */
typedef struct {
    IR_DISTANCE_SensorType_t sensorType; /**< Sensor type */
    uint16_t minDistance;                /**< Minimum measurable distance in mm */
    uint16_t maxDistance;                /**< Maximum measurable distance in mm */
    uint16_t averagingSamples;           /**< Number of samples for averaging */
    uint32_t measurementTimeout;         /**< Timeout for a whole averaged measurement, ms */
} IR_DISTANCE_Config_t;

/**
 * @brief IR distance sensor calibration point
 */
typedef struct {
    uint16_t distance; /**< Distance in mm */
    uint16_t adcValue; /**< Corresponding ADC value */
} IR_DISTANCE_CalibrationPoint_t;

/**
 * @brief IR distance sensor calibration curve
 * @note  Points must be sorted by increasing distance and, because the Sharp
 *        output voltage falls as the target recedes, by strictly decreasing
 *        ADC value. The interpolation depends on both orderings.
 */
typedef struct {
    uint8_t numPoints; /**< Number of calibration points */
    IR_DISTANCE_CalibrationPoint_t points[IR_DISTANCE_MAX_CURVE_POINTS];
} IR_DISTANCE_CustomCurve_t;

/**
 * @brief IR distance sensor handle structure
 */
typedef struct {
    ADC_HandleStruct *hadc;                /**< ADC handle */
    uint32_t channel;                      /**< ADC channel */
    IR_DISTANCE_Config_t config;           /**< Sensor configuration */
    IR_DISTANCE_CustomCurve_t customCurve; /**< Active calibration curve */

    uint16_t lastAdcValue; /**< Last averaged ADC reading */
    uint16_t lastDistance; /**< Last valid distance in mm, 0 if none */
    bool isInitialized;    /**< Initialization status */
} IR_DISTANCE_Handle_t;

/* Exported macros -----------------------------------------------------------*/

/**
 * @brief Common entry guard for every public entry point taking a handle.
 */
#define IR_DISTANCE_CHECK_HANDLE(h)                                                                \
    do {                                                                                           \
        if ((h) == NULL) {                                                                         \
            return IR_DISTANCE_INVALID_PARAM;                                                      \
        }                                                                                          \
        if (!(h)->isInitialized) {                                                                 \
            return IR_DISTANCE_NOT_INITIALIZED;                                                    \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* IR_DISTANCE_TYPES_H */
