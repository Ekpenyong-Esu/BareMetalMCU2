/**
 ******************************************************************************
 * @file    dht_decode.c
 * @brief   DHT frame validation and conversion
 ******************************************************************************
 */

#include "dht_decode.h"

#include <stddef.h>

/* DHT22 fields are tenths; temperature is sign-magnitude with bit 15 the sign */
#define DHT22_SCALE 10.0f
#define DHT22_TEMP_MAGNITUDE_MASK 0x7FFFU
#define DHT22_TEMP_SIGN_MASK 0x8000U

#define DHT_BYTE_SHIFT 8U

bool DHT_Decode_ChecksumOk(const uint8_t *frame) {
    if (frame == NULL) {
        return false;
    }

    uint8_t sum = (uint8_t)(frame[0] + frame[1] + frame[2] + frame[3]);

    return sum == frame[4];
}

void DHT_Decode_Sample(DHT_Type_t type, const uint8_t *frame, float *humidity,
                       float *temperatureC) {
    if (frame == NULL || humidity == NULL || temperatureC == NULL) {
        return;
    }

    if (type == DHT_TYPE_DHT11) {
        *humidity = (float)frame[0];
        *temperatureC = (float)frame[2];
        return;
    }

    uint16_t rawHum = (uint16_t)(((uint16_t)frame[0] << DHT_BYTE_SHIFT) | frame[1]);
    uint16_t rawTemp = (uint16_t)(((uint16_t)frame[2] << DHT_BYTE_SHIFT) | frame[3]);

    *humidity = (float)rawHum / DHT22_SCALE;

    /* DHT22 temperature is sign-magnitude, not two's complement. */
    float tempC = (float)(rawTemp & DHT22_TEMP_MAGNITUDE_MASK) / DHT22_SCALE;
    *temperatureC = ((rawTemp & DHT22_TEMP_SIGN_MASK) != 0U) ? -tempC : tempC;
}
