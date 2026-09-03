/**
  ******************************************************************************
  * @file    dht.h
  * @brief   DHT11 / DHT22 temperature and humidity sensor driver
  * @details Simple driver for the DHT family. One wire is used for both
  *          sending the start signal and reading the 40-bit reply. The
  *          sensor is slow, so the driver enforces a gap between reads
  *          (1 s for DHT11, 2 s for DHT22).
  *
  * How it works (in simple words):
  * - The MCU pulls the data line low to wake the sensor.
  * - The sensor replies with 40 bits: humidity + temperature + checksum.
  * - Each bit is coded by how long the line stays high (short = 0, long = 1).
  * - The driver checks the checksum and saves humidity and temperature.
  */

#ifndef DHT_H
#define DHT_H

#include "stm32f4xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Which DHT sensor is connected
 */
typedef enum {
    DHT_TYPE_DHT11 = 0,  /*!< Cheap sensor, integer degrees only */
    DHT_TYPE_DHT22       /*!< More accurate, one decimal place */
} DHT_Type_t;

/**
 * @brief Holds everything for one DHT sensor
 */
typedef struct {
    DHT_Type_t type;        /*!< DHT11 or DHT22 */
    GPIO_TypeDef *port;     /*!< GPIO port the data line is on */
    uint16_t pin;           /*!< GPIO pin for the data line */
    uint32_t lastReadMs;    /*!< When the last read was done (HAL tick) */
    bool valid;             /*!< True if the last read passed the checksum */
    float humidity;         /*!< Last humidity in % (0..100) */
    float temperatureC;     /*!< Last temperature in degrees C */
} DHT_Handle_t;

/**
 * @brief Set up a DHT sensor
 * @param hdht  Handle to fill in (must not be NULL)
 * @param type  DHT11 or DHT22
 * @param port  GPIO port for the data line
 * @param pin   GPIO pin for the data line
 * @retval HAL_OK if ready, HAL_ERROR if something is wrong
 */
HAL_StatusTypeDef DHT_Init(DHT_Handle_t *hdht, DHT_Type_t type, GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Read humidity and temperature from the sensor
 * @param hdht  Handle (must be initialized)
 * @retval HAL_OK on success, HAL_BUSY if called too soon, HAL_ERROR/TIMEOUT on failure
 * @note  Waits at least 1 s (DHT11) or 2 s (DHT22) between reads
 */
HAL_StatusTypeDef DHT_Read(DHT_Handle_t *hdht);

/**
 * @brief Check if the last reading is good
 * @param hdht  Handle
 * @retval true if the last read passed the checksum
 */
bool DHT_IsValid(const DHT_Handle_t *hdht);

/**
 * @brief Get the last humidity value
 * @param hdht  Handle
 * @retval Humidity in % (0 if handle is NULL)
 */
float DHT_GetHumidity(const DHT_Handle_t *hdht);

/**
 * @brief Get the last temperature value
 * @param hdht  Handle
 * @retval Temperature in degrees C (0 if handle is NULL)
 */
float DHT_GetTemperatureC(const DHT_Handle_t *hdht);

#ifdef __cplusplus
}
#endif

#endif /* DHT_H */
