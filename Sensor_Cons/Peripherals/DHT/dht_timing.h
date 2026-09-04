/**
 ******************************************************************************
 * @file    dht_timing.h
 * @brief   Microsecond time base for the DHT driver - internal
 * @details Wraps the Cortex-M4 DWT cycle counter. Knows nothing about the
 *          sensor or the wire; it only answers "how long since" and "wait".
 *          Not part of dht.h.
 ******************************************************************************
 */

#ifndef DHT_TIMING_H
#define DHT_TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Start the cycle counter used for every delay and timeout.
 * @retval  false if the counter will not run, which would hang the spin loops.
 */
bool DHT_Timing_Init(void);

/** @brief Free-running tick reading; only differences are meaningful. */
uint32_t DHT_Timing_Now(void);

/** @brief Microseconds elapsed since a reading taken by DHT_Timing_Now(). */
uint32_t DHT_Timing_ElapsedUs(uint32_t startTicks);

/** @brief Busy-wait, safe to call with interrupts masked. */
void DHT_Timing_DelayUs(uint32_t micros);

#ifdef __cplusplus
}
#endif

#endif /* DHT_TIMING_H */
