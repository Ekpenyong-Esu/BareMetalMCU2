/**
 * @file rng_core.h
 * @brief RNG peripheral lifetime, health and status reporting
 */

#ifndef RNG_CORE_H
#define RNG_CORE_H

#include "rng_types.h"

/**
 * @brief Initialize the Random Number Generator
 * @note The RNG is clocked from PLL48CLK, which SystemClock_Config sets up.
 */
RNG_StatusTypeDef RNG_Init(void);

/**
 * @brief Deinitialize the Random Number Generator
 */
RNG_StatusTypeDef RNG_DeInit(void);

/**
 * @brief Whether RNG_Init has completed successfully
 */
bool RNG_IsInitialized(void);

/**
 * @brief HAL handle backing the driver, NULL until initialized
 * @note Exposed so the HASH/RNG vector in stm32f4xx_it.c can reach it.
 */
RNG_HandleTypeDef *RNG_GetHandle(void);

/**
 * @brief Check if a random word is available
 */
bool RNG_IsReady(void);

/**
 * @brief Current seed or clock error state
 */
RNG_StatusTypeDef RNG_GetErrorStatus(void);

/**
 * @brief Whether a seed or clock error is currently latched
 */
bool RNG_HasError(void);

/**
 * @brief Clear the latched error flags and restart the generator
 */
RNG_StatusTypeDef RNG_ClearErrors(void);

/**
 * @brief Convert a status code to a human-readable string
 */
const char *RNG_GetStatusString(RNG_StatusTypeDef status);

#endif /* RNG_CORE_H */
