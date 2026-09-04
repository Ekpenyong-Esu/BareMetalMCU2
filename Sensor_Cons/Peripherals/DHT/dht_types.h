/**
 ******************************************************************************
 * @file    dht_types.h
 * @brief   DHT11 / DHT22 driver data definitions
 * @details Shared vocabulary only, no behaviour, so every module can include
 *          it without a dependency cycle.
 ******************************************************************************
 */

#ifndef DHT_TYPES_H
#define DHT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/** Frame: humidity (2 bytes), temperature (2 bytes), checksum (1 byte) */
#define DHT_FRAME_BYTES 5U
#define DHT_FRAME_BITS 40U

/** Minimum gap between reads: DHT11 1 s, DHT22 2 s recommended */
#define DHT11_MIN_INTERVAL_MS 1000U
#define DHT22_MIN_INTERVAL_MS 2000U

/**
 * @brief Which DHT sensor is connected
 */
typedef enum {
    DHT_TYPE_DHT11 = 0, /*!< Cheap sensor, integer degrees only */
    DHT_TYPE_DHT22      /*!< More accurate, one decimal place */
} DHT_Type_t;

/**
 * @brief Holds everything for one DHT sensor
 */
typedef struct {
    DHT_Type_t type;     /*!< DHT11 or DHT22 */
    GPIO_TypeDef *port;  /*!< GPIO port the data line is on */
    uint16_t pin;        /*!< GPIO pin for the data line */
    uint32_t lastReadMs; /*!< When the last read was done (HAL tick) */
    bool valid;          /*!< True if the last read passed the checksum */
    float humidity;      /*!< Last humidity in % (0..100) */
    float temperatureC;  /*!< Last temperature in degrees C */
} DHT_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* DHT_TYPES_H */
