/**
 * @file rng_generate.h
 * @brief Raw random word and byte generation
 */

#ifndef RNG_GENERATE_H
#define RNG_GENERATE_H

#include "rng_types.h"

/**
 * @brief Generate a 32-bit random number
 */
RNG_StatusTypeDef RNG_Generate(uint32_t *randomNumber);

/**
 * @brief Fill a buffer with random 32-bit values
 */
RNG_StatusTypeDef RNG_GenerateBuffer(uint32_t *buffer, uint32_t count);

/**
 * @brief Fill a buffer with random bytes
 */
RNG_StatusTypeDef RNG_GenerateBytes(uint8_t *buffer, uint32_t length);

#endif /* RNG_GENERATE_H */
