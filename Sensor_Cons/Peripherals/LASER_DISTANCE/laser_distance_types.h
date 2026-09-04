/**
 ******************************************************************************
 * @file    laser_distance_types.h
 * @brief   Laser distance sensor data types and constants
 * @details Status codes, sensor types, configuration and measurement records.
 *          Contains no behaviour.
 ******************************************************************************
 */

#ifndef LASER_DISTANCE_TYPES_H
#define LASER_DISTANCE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "i2c_types.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Laser distance sensor status enumeration
 */
typedef enum {
    LASER_DISTANCE_OK = 0,          /**< Operation completed successfully */
    LASER_DISTANCE_ERROR,           /**< General error occurred */
    LASER_DISTANCE_BUSY,            /**< Sensor is busy measuring */
    LASER_DISTANCE_TIMEOUT,         /**< Measurement timeout */
    LASER_DISTANCE_INVALID_PARAM,   /**< Invalid parameter provided */
    LASER_DISTANCE_NOT_INITIALIZED, /**< Driver not initialized */
    LASER_DISTANCE_OUT_OF_RANGE,    /**< Distance out of sensor range */
    LASER_DISTANCE_I2C_ERROR        /**< I2C communication error */
} LASER_DISTANCE_StatusTypeDef;

/**
 * @brief Laser distance sensor type enumeration
 */
typedef enum {
    LASER_DISTANCE_VL53L0X = 0, /**< ST VL53L0X (30-2000mm) */
    LASER_DISTANCE_VL53L1X,     /**< ST VL53L1X (40-4000mm) */
    LASER_DISTANCE_TFMINI,      /**< Benewake TFmini (30-12000mm) */
    LASER_DISTANCE_CUSTOM       /**< Custom sensor with user-defined protocol */
} LASER_DISTANCE_SensorType_t;

/**
 * @brief Laser distance sensor configuration structure
 */
typedef struct {
    LASER_DISTANCE_SensorType_t sensorType; /**< Sensor type */
    uint16_t minDistance;                   /**< Minimum measurable distance in mm */
    uint16_t maxDistance;                   /**< Maximum measurable distance in mm */
    uint16_t averagingSamples;              /**< Reserved: no averaging is performed yet */
    uint32_t measurementTimeout;            /**< Measurement timeout in ms */
    uint8_t i2cAddress;                     /**< I2C slave address, 8-bit (shifted) form */
} LASER_DISTANCE_Config_t;

/**
 * @brief Laser distance sensor measurement data structure
 */
typedef struct {
    uint16_t distance;    /**< Measured distance in mm */
    uint16_t ambientRate; /**< Ambient light rate (VL53L0X) */
    uint16_t signalRate;  /**< Signal rate (VL53L0X) */
    uint8_t rangeStatus;  /**< Range status (VL53L0X) */
    uint32_t timestamp;   /**< Measurement timestamp */
} LASER_DISTANCE_Measurement_t;

/**
 * @brief Laser distance sensor handle structure
 * @note  The bus is opened by the application; the driver registers this
 *        sensor's device record on it during LASER_DISTANCE_Init.
 */
typedef struct {
    I2C_Device_t device;            /**< This sensor's record on the caller's bus */
    LASER_DISTANCE_Config_t config; /**< Sensor configuration */

    LASER_DISTANCE_Measurement_t lastMeasurement; /**< Last measurement data */
    bool isInitialized;                           /**< Initialization status */
    bool isMeasuring;                             /**< Continuous measurement active */
} LASER_DISTANCE_Handle_t;

/* Exported constants --------------------------------------------------------*/

/* Default configuration values */
#define LASER_DISTANCE_DEFAULT_AVERAGING_SAMPLES 5U
#define LASER_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT 500U /* 500ms */
#define LASER_DISTANCE_DEFAULT_I2C_ADDRESS 0x52U        /* VL53L0X default */

/* Sensor specific ranges (mm) */
#define LASER_DISTANCE_VL53L0X_MIN 30U   /* 3cm */
#define LASER_DISTANCE_VL53L0X_MAX 2000U /* 200cm */
#define LASER_DISTANCE_VL53L1X_MIN 40U   /* 4cm */
#define LASER_DISTANCE_VL53L1X_MAX 4000U /* 400cm */
#define LASER_DISTANCE_TFMINI_MIN 30U    /* 3cm */
#define LASER_DISTANCE_TFMINI_MAX 12000U /* 1200cm */

/* Fallback range for CUSTOM and unknown sensor types (mm) */
#define LASER_DISTANCE_CUSTOM_MIN 10U
#define LASER_DISTANCE_CUSTOM_MAX 1000U

/* Configuration limits */
#define LASER_DISTANCE_MAX_AVERAGING_SAMPLES 100U
#define LASER_DISTANCE_MIN_7BIT_ADDRESS 0x08U
#define LASER_DISTANCE_MAX_7BIT_ADDRESS 0x77U

/* I2C transfer timeout in ms */
#define LASER_DISTANCE_I2C_TIMEOUT 100U

#ifdef __cplusplus
}
#endif

#endif /* LASER_DISTANCE_TYPES_H */
