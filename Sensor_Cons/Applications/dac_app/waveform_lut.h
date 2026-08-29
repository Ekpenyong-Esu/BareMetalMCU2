/**
 * @file waveform_lut.h
 * @brief Precomputed waveform lookup tables (sine, triangle, sawtooth)
 */

#ifndef WAVEFORM_LUT_H
#define WAVEFORM_LUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Points per waveform cycle. */
#ifndef WAVEFORM_LUT_SIZE
#define WAVEFORM_LUT_SIZE 100U
#endif

typedef enum {
    WAVEFORM_SINE = 0,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_TYPE_COUNT
} Waveform_Type_t;

/** Build one full cycle of @p type (0..full scale) into @p lut. */
void WaveformLut_Build(uint32_t *lut, Waveform_Type_t type);

/** Human-readable name, for logging. */
const char *WaveformLut_Name(Waveform_Type_t type);

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_LUT_H */
