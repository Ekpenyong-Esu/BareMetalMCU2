/**
 * @file rng_async.h
 * @brief Interrupt-driven random number generation
 * @note  Requires HASH_RNG_IRQHandler in stm32f4xx_it.c to call RNG_IRQHandler.
 */

#ifndef RNG_ASYNC_H
#define RNG_ASYNC_H

#include "rng_types.h"

/**
 * @brief Enable the HASH/RNG interrupt line
 */
RNG_StatusTypeDef RNG_EnableInterrupt(void);

/**
 * @brief Disable the HASH/RNG interrupt line and drop any pending request
 */
RNG_StatusTypeDef RNG_DisableInterrupt(void);

/**
 * @brief Request one random word, delivered to @p callback from the ISR
 * @note  Only one request can be outstanding at a time.
 */
RNG_StatusTypeDef RNG_GenerateAsync(RNG_DataCallback_t callback);

/**
 * @brief Whether an asynchronous request is still outstanding
 */
bool RNG_IsGenerationPending(void);

/**
 * @brief Service routine, called from the HASH/RNG vector
 */
void RNG_IRQHandler(void);

#endif /* RNG_ASYNC_H */
