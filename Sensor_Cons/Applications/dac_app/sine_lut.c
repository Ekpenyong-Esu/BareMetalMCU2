/**
 * @file sine_lut.c
 * @brief Sine lookup table generator
 */

#include "sine_lut.h"

#include "dac.h"
#include <math.h>
#include <stdint.h>

void SineLut_Build(uint32_t *lut)
{
    for (uint32_t i = 0; i < SINE_LUT_SIZE; i++) {
        float angle = (2.0f * (float)M_PI * (float)i) / (float)SINE_LUT_SIZE;
        float unit  = (sinf(angle) + 1.0f) * 0.5f; /* rescale -1..1 to 0..1 */
        lut[i]      = (uint32_t)(unit * (float)DAC_MAX_VALUE_12BIT);
    }
}
