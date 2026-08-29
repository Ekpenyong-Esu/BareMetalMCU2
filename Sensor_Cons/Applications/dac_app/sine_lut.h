/**
 * @file sine_lut.h
 * @brief Precomputed sine lookup table
 */

#ifndef SINE_LUT_H
#define SINE_LUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Points per sine cycle (100 by default). */
#ifndef SINE_LUT_SIZE
#define SINE_LUT_SIZE 100U
#endif

/** Build one full-cycle sine LUT (0..full scale) inside @p lut. */
void SineLut_Build(uint32_t *lut);

#ifdef __cplusplus
}
#endif

#endif /* SINE_LUT_H */
