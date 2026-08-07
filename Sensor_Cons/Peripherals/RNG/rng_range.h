/**
 * @file rng_range.h
 * @brief Bounded and typed random values derived from the raw generator
 */

#ifndef RNG_RANGE_H
#define RNG_RANGE_H

#include "rng_types.h"

/**
 * @brief Generate a uniformly distributed value in [0, max)
 */
RNG_StatusTypeDef RNG_GenerateRange(uint32_t max, uint32_t *result);

/**
 * @brief Generate a uniformly distributed value in [min, max]
 */
RNG_StatusTypeDef RNG_GenerateInRange(uint32_t min, uint32_t max, uint32_t *result);

/**
 * @brief Generate a random boolean
 */
RNG_StatusTypeDef RNG_GenerateBool(bool *result);

/**
 * @brief Generate a random float in [0.0, 1.0)
 */
RNG_StatusTypeDef RNG_GenerateFloat(float *result);

#endif /* RNG_RANGE_H */
