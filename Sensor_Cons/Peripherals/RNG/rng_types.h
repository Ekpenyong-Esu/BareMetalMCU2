/**
 * @file rng_types.h
 * @brief Shared vocabulary for the hardware RNG driver
 */

#ifndef RNG_TYPES_H
#define RNG_TYPES_H

#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief RNG Status enumeration
 */
typedef enum {
    RNG_OK = 0,            /**< Operation completed successfully */
    RNG_ERROR,             /**< General error occurred */
    RNG_TIMEOUT,           /**< Operation timed out */
    RNG_CLOCK_ERROR,       /**< Clock configuration error */
    RNG_SEED_ERROR,        /**< Seed error detected, the current word is invalid */
    RNG_BUSY,              /**< An asynchronous generation is already in flight */
    RNG_NOT_READY          /**< RNG not initialized */
} RNG_StatusTypeDef;

/**
 * @brief Delivery callback for RNG_GenerateAsync
 * @note Runs in interrupt context.
 */
typedef void (*RNG_DataCallback_t)(uint32_t randomNumber);

/**
 * @brief Attempts allowed when rejection sampling a bounded value
 * @note Each draw is rejected with probability below 1/2, so exhausting this
 *       budget means the peripheral is misbehaving, not that we were unlucky.
 */
#define RNG_RANGE_MAX_ATTEMPTS  64U

/** Bits of a float mantissa that can hold an exact integer */
#define RNG_FLOAT_MANTISSA_BITS 24U
#define RNG_FLOAT_DIVISOR       16777216.0f  /**< 2^24 */

/** Priority of the shared HASH/RNG interrupt */
#define RNG_IRQ_PRIORITY        5U
#define RNG_IRQ_SUBPRIORITY     0U

#endif /* RNG_TYPES_H */
