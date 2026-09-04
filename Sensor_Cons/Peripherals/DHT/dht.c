/**
 ******************************************************************************
 * @file    dht.c
 * @brief   DHT lifecycle, acquisition and last-sample access
 * @details Sequences one read: enforce the gap, fetch a frame over the wire,
 *          check it, convert it, publish it. Each of those steps belongs to a
 *          module below this one.
 ******************************************************************************
 */

#include "dht_core.h"

#include "dht_bus.h"
#include "dht_decode.h"
#include "dht_timing.h"

#include <stddef.h>

static uint32_t DHT_MinIntervalMs(DHT_Type_t type) {
    return (type == DHT_TYPE_DHT11) ? DHT11_MIN_INTERVAL_MS : DHT22_MIN_INTERVAL_MS;
}

static uint32_t DHT_StartPulseMs(DHT_Type_t type) {
    return (type == DHT_TYPE_DHT11) ? DHT11_START_PULSE_MS : DHT22_START_PULSE_MS;
}

HAL_StatusTypeDef DHT_Init(DHT_Handle_t *hdht, DHT_Type_t type, GPIO_TypeDef *port, uint16_t pin) {
    if (hdht == NULL || port == NULL) {
        return HAL_ERROR;
    }

    if (!DHT_Timing_Init()) {
        return HAL_ERROR;
    }

    hdht->type = type;
    hdht->port = port;
    hdht->pin = pin;
    hdht->lastReadMs = 0U;
    hdht->valid = false;
    hdht->humidity = 0.0f;
    hdht->temperatureC = 0.0f;

    DHT_Bus_Release(port, pin);

    return HAL_OK;
}

HAL_StatusTypeDef DHT_Read(DHT_Handle_t *hdht) {
    if (hdht == NULL) {
        return HAL_ERROR;
    }

    uint32_t now = HAL_GetTick();
    if ((now - hdht->lastReadMs) < DHT_MinIntervalMs(hdht->type)) {
        return HAL_BUSY;
    }

    /* Any early return below leaves no fresh sample. Failed attempts are
       rate-limited too, so a missing sensor is not hammered. */
    hdht->valid = false;
    hdht->lastReadMs = now;

    uint8_t frame[DHT_FRAME_BYTES];

    HAL_StatusTypeDef status =
        DHT_Bus_ReadFrame(hdht->port, hdht->pin, DHT_StartPulseMs(hdht->type), frame);
    if (status != HAL_OK) {
        return status;
    }

    if (!DHT_Decode_ChecksumOk(frame)) {
        return HAL_ERROR;
    }

    DHT_Decode_Sample(hdht->type, frame, &hdht->humidity, &hdht->temperatureC);
    hdht->valid = true;

    return HAL_OK;
}

bool DHT_IsValid(const DHT_Handle_t *hdht) {
    return (hdht != NULL) && hdht->valid;
}

float DHT_GetHumidity(const DHT_Handle_t *hdht) {
    return (hdht != NULL) ? hdht->humidity : 0.0f;
}

float DHT_GetTemperatureC(const DHT_Handle_t *hdht) {
    return (hdht != NULL) ? hdht->temperatureC : 0.0f;
}
