/**
 ******************************************************************************
 * @file    dht_decode.h
 * @brief   DHT frame validation and conversion
 * @details Turns the five raw bytes into humidity and temperature. Touches no
 *          hardware and keeps no state, so it can be unit tested on its own.
 ******************************************************************************
 */

#ifndef DHT_DECODE_H
#define DHT_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dht_types.h"

/**
 * @brief   Check the frame's trailing checksum byte against the four before it.
 */
bool DHT_Decode_ChecksumOk(const uint8_t *frame);

/**
 * @brief   Convert a checksum-verified frame into engineering units.
 * @param   type          Chooses the encoding: DHT11 is integer, DHT22 tenths.
 * @param   humidity      Out: relative humidity in %.
 * @param   temperatureC  Out: temperature in degrees C, negatives included.
 */
void DHT_Decode_Sample(DHT_Type_t type, const uint8_t *frame, float *humidity, float *temperatureC);

#ifdef __cplusplus
}
#endif

#endif /* DHT_DECODE_H */
