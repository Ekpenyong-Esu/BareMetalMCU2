/**
 ******************************************************************************
 * @file    dht_core.h
 * @brief   DHT lifecycle, acquisition and last-sample access
 * @details The public API. Owns the handle and the read policy (how often the
 *          sensor may be asked); the wire, the clock and the maths live in
 *          dht_bus.h, dht_timing.h and dht_decode.h.
 ******************************************************************************
 */

#ifndef DHT_CORE_H
#define DHT_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dht_types.h"

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
 * @retval true if the last read passed the checksum
 */
bool DHT_IsValid(const DHT_Handle_t *hdht);

/**
 * @brief Get the last humidity value
 * @retval Humidity in % (0 if handle is NULL)
 */
float DHT_GetHumidity(const DHT_Handle_t *hdht);

/**
 * @brief Get the last temperature value
 * @retval Temperature in degrees C (0 if handle is NULL)
 */
float DHT_GetTemperatureC(const DHT_Handle_t *hdht);

#ifdef __cplusplus
}
#endif

#endif /* DHT_CORE_H */
