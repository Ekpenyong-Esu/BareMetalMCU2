/**
 ******************************************************************************
 * @file    accel_convert.c
 * @brief   Raw-sample decoding and unit conversion for the MMA8452Q
 ******************************************************************************
 */

#include "accel_convert.h"
#include "accel_mma8452q.h"

/* Counts per g for each ACCEL_RANGE_*: 2^14 divided by the full-scale span. */
static const float s_sensitivity[] = {
    [ACCEL_RANGE_2G] = 4096.0f,
    [ACCEL_RANGE_4G] = 2048.0f,
    [ACCEL_RANGE_8G] = 1024.0f,
};

int16_t ACCEL_ConvertRawToSigned(uint8_t msb, uint8_t lsb) {
    int16_t raw = (int16_t)(((uint16_t)msb << 8) | (uint16_t)lsb);

    /* The sample is left-aligned in 16 bits; drop the two unused LSBs. */
    raw >>= 2;

    if (raw & ACCEL_SIGN_BIT_14) {
        raw |= (int16_t)ACCEL_SIGN_EXTEND_14;
    }

    return raw;
}

float ACCEL_ConvertToG(int16_t raw, uint8_t range) {
    float sensitivity = s_sensitivity[ACCEL_RANGE_2G];

    if (range < (sizeof(s_sensitivity) / sizeof(s_sensitivity[0]))) {
        sensitivity = s_sensitivity[range];
    }

    return (float)raw / sensitivity;
}
