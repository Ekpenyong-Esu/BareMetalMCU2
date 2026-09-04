/**
 ******************************************************************************
 * @file    accel_types.h
 * @brief   Shared vocabulary for the MMA8452Q accelerometer driver
 ******************************************************************************
 */

#ifndef ACCEL_TYPES_H
#define ACCEL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "stm32f4xx_hal.h"
#include "spi_types.h"

/* Status --------------------------------------------------------------------*/
typedef enum {
    ACCEL_OK = 0,        /**< Operation completed successfully */
    ACCEL_ERROR,         /**< General error occurred */
    ACCEL_BUSY,          /**< Accelerometer is busy */
    ACCEL_TIMEOUT,       /**< Operation timed out */
    ACCEL_INVALID_PARAM, /**< Invalid parameter provided */
    ACCEL_NOT_READY      /**< Device not ready */
} ACCEL_StatusTypeDef;

/* Output data rates ---------------------------------------------------------*/
#define ACCEL_ODR_800HZ 0x00U
#define ACCEL_ODR_400HZ 0x01U
#define ACCEL_ODR_200HZ 0x02U
#define ACCEL_ODR_100HZ 0x03U
#define ACCEL_ODR_50HZ 0x04U
#define ACCEL_ODR_12_5HZ 0x05U
#define ACCEL_ODR_6_25HZ 0x06U
#define ACCEL_ODR_1_56HZ 0x07U

/* Measurement ranges --------------------------------------------------------*/
#define ACCEL_RANGE_2G 0x00U
#define ACCEL_RANGE_4G 0x01U
#define ACCEL_RANGE_8G 0x02U

/* Operating modes -----------------------------------------------------------*/
#define ACCEL_MODE_STANDBY 0x00U
#define ACCEL_MODE_ACTIVE 0x01U
#define ACCEL_MODE_SLEEP 0x02U

/* Timeouts ------------------------------------------------------------------*/
#define ACCEL_TIMEOUT_DEFAULT 1000U
#define ACCEL_TIMEOUT_INIT 5000U
#define ACCEL_SPI_TIMEOUT 100U

/* Configuration -------------------------------------------------------------*/
typedef struct {
    uint8_t DataRate;    /**< One of ACCEL_ODR_* */
    uint8_t Range;       /**< One of ACCEL_RANGE_* */
    uint8_t Mode;        /**< One of ACCEL_MODE_* */
    bool HighPassFilter; /**< High-pass filter enable */
    bool LowNoise;       /**< Low noise mode enable */
} ACCEL_ConfigTypeDef;

/* Sample --------------------------------------------------------------------*/
typedef struct {
    int16_t X; /**< X-axis acceleration (raw, 14-bit signed) */
    int16_t Y;
    int16_t Z;
    float X_g; /**< X-axis acceleration in g */
    float Y_g;
    float Z_g;
} ACCEL_DataTypeDef;

/* Interrupt configuration ---------------------------------------------------*/
typedef struct {
    bool DataReady;
    bool Motion;
    bool Freefall;
    bool Tap;
} ACCEL_IntConfigTypeDef;

/* Handle --------------------------------------------------------------------*/

/**
 * @brief One MMA8452Q and the wiring the application gave it.
 * @note  NSS is software-driven, so the part is only addressed while the
 *        chip-select pin is low; the bus itself is shared and caller-owned.
 */
typedef struct {
    SPI_Device_t device;        /**< Bus settings this part needs */
    GPIO_TypeDef *csPort;       /**< Chip-select port */
    uint16_t csPin;             /**< Chip-select pin */
    ACCEL_ConfigTypeDef config; /**< Settings applied at init */
    uint8_t cachedRange;        /**< Last range written, for scaling samples */
    bool isInitialized;
} ACCEL_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_TYPES_H */
