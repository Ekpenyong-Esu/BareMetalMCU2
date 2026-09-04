/**
 * @file rng_range.c
 * @brief Bounded and typed random values derived from the raw generator
 */

#include "rng_range.h"
#include "rng_generate.h"

/** Width of one hardware random word */
#define RNG_WORD_BITS 32U

RNG_StatusTypeDef RNG_GenerateRange(uint32_t max, uint32_t *result) {
    uint32_t remainder = 0;
    uint32_t acceptLimit = 0;

    if (result == NULL || max == 0U) {
        return RNG_ERROR;
    }

    if (max == 1U) {
        *result = 0U;
        return RNG_OK;
    }

    /* Plain `word % max` over-represents the low residues whenever max does not
       divide 2^32, so draws landing in the short final block are rejected.
       remainder is 2^32 mod max, computed without a 64-bit divide. */
    remainder = (0U - max) % max;
    acceptLimit = UINT32_MAX - remainder;

    for (uint32_t attempt = 0; attempt < RNG_RANGE_MAX_ATTEMPTS; attempt++) {
        uint32_t randomNumber = 0;
        RNG_StatusTypeDef status = RNG_Generate(&randomNumber);

        if (status != RNG_OK) {
            return status;
        }

        if (randomNumber <= acceptLimit) {
            *result = randomNumber % max;
            return RNG_OK;
        }
    }

    return RNG_TIMEOUT;
}

RNG_StatusTypeDef RNG_GenerateInRange(uint32_t min, uint32_t max, uint32_t *result) {
    uint32_t range = 0;
    uint32_t randomValue = 0;
    RNG_StatusTypeDef status = RNG_OK;

    if (result == NULL || min > max) {
        return RNG_ERROR;
    }

    if (min == max) {
        *result = min;
        return RNG_OK;
    }

    range = max - min + 1U;
    if (range == 0U) {
        /* The full 32-bit span; every raw word is already in range. */
        return RNG_Generate(result);
    }

    status = RNG_GenerateRange(range, &randomValue);
    if (status != RNG_OK) {
        return status;
    }

    *result = min + randomValue;

    return RNG_OK;
}

RNG_StatusTypeDef RNG_GenerateBool(bool *result) {
    uint32_t randomNumber = 0;
    RNG_StatusTypeDef status = RNG_OK;

    if (result == NULL) {
        return RNG_ERROR;
    }

    status = RNG_Generate(&randomNumber);
    if (status != RNG_OK) {
        return status;
    }

    *result = ((randomNumber & 0x01U) != 0U);

    return RNG_OK;
}

RNG_StatusTypeDef RNG_GenerateFloat(float *result) {
    uint32_t randomNumber = 0;
    RNG_StatusTypeDef status = RNG_OK;

    if (result == NULL) {
        return RNG_ERROR;
    }

    status = RNG_Generate(&randomNumber);
    if (status != RNG_OK) {
        return status;
    }

    /* Only 24 bits fit exactly in a float; dividing the full word by 2^32
       rounds the top values up to exactly 1.0f, outside the documented range. */
    *result =
        (float)(randomNumber >> (RNG_WORD_BITS - RNG_FLOAT_MANTISSA_BITS)) / RNG_FLOAT_DIVISOR;

    return RNG_OK;
}
