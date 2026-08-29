/**
 * @file waveform_lut.c
 * @brief Waveform lookup table generators
 */

#include "waveform_lut.h"

#include "dac.h"

#include <math.h>

#define FULL_SCALE  ((float)DAC_MAX_VALUE_12BIT)
#define LAST_INDEX  ((float)(WAVEFORM_LUT_SIZE - 1U))

/* Sample i is the code for angle 2*pi*i/N, shifted from -1..1 into 0..1. */
static void BuildSine(uint32_t *lut)
{
    for (uint32_t i = 0; i < WAVEFORM_LUT_SIZE; i++) {
        float angle = (2.0f * (float)M_PI * (float)i) / (float)WAVEFORM_LUT_SIZE;
        float unit  = (sinf(angle) + 1.0f) * 0.5f;
        lut[i]      = (uint32_t)(unit * FULL_SCALE);
    }
}

/* Ramps 0 -> full scale over the first half, back down over the second. */
static void BuildTriangle(uint32_t *lut)
{
    const uint32_t half = WAVEFORM_LUT_SIZE / 2U;

    for (uint32_t i = 0; i < WAVEFORM_LUT_SIZE; i++) {
        float unit = (i < half) ? ((float)i / (float)half)
                                : (1.0f - ((float)(i - half) / (float)(WAVEFORM_LUT_SIZE - half)));
        lut[i] = (uint32_t)(unit * FULL_SCALE);
    }
}

/* Ramps 0 -> full scale across the cycle, then snaps back on wrap. */
static void BuildSawtooth(uint32_t *lut)
{
    for (uint32_t i = 0; i < WAVEFORM_LUT_SIZE; i++) {
        lut[i] = (uint32_t)(((float)i / LAST_INDEX) * FULL_SCALE);
    }
}

void WaveformLut_Build(uint32_t *lut, Waveform_Type_t type)
{
    switch (type) {
        case WAVEFORM_TRIANGLE:
            BuildTriangle(lut);
            break;
        case WAVEFORM_SAWTOOTH:
            BuildSawtooth(lut);
            break;
        case WAVEFORM_SINE:
        default:
            BuildSine(lut);
            break;
    }
}

const char *WaveformLut_Name(Waveform_Type_t type)
{
    switch (type) {
        case WAVEFORM_TRIANGLE: return "triangle";
        case WAVEFORM_SAWTOOTH: return "sawtooth";
        case WAVEFORM_SINE:     return "sine";
        default:                return "unknown";
    }
}
