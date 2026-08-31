/**
 * @file ir_timing.h
 * @brief IR timing conversions and microsecond delay
 */

#ifndef IR_TIMING_H
#define IR_TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_types.h"

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Enable the DWT cycle counter used by IR_DelayUs()
 * @return void
 */
void IR_TimingInit(void);

/**
 * @brief Report whether the DWT cycle counter is actually advancing
 * @note  IR_DelayUs() spins on it, so a stalled counter would hang the driver.
 */
bool IR_TimingIsRunning(void);

/**
 * @brief Busy-wait for a number of microseconds
 * @note  IR pulses are tens of microseconds long, so HAL_Delay() with its 1 ms
 *        granularity cannot be used here.
 * @param us: Delay in microseconds
 * @return void
 */
void IR_DelayUs(uint32_t us);

/**
 * @brief Convert microseconds to timer ticks
 * @param microseconds: Time in microseconds
 * @param timerFreq: Timer frequency in Hz
 * @return uint32_t: Timer ticks, 0 when timerFreq is 0
 */
uint32_t IR_MicrosecondsToTicks(uint32_t microseconds, uint32_t timerFreq);

/**
 * @brief Convert timer ticks to microseconds
 * @param ticks: Timer ticks
 * @param timerFreq: Timer frequency in Hz
 * @return uint32_t: Time in microseconds, 0 when timerFreq is 0
 */
uint32_t IR_TicksToMicroseconds(uint32_t ticks, uint32_t timerFreq);

/**
 * @brief Check if timing is within tolerance
 * @param measured: Measured time
 * @param expected: Expected time
 * @param tolerance: Tolerance value
 * @return bool: True if within tolerance
 */
bool IR_IsWithinTolerance(uint32_t measured, uint32_t expected, uint32_t tolerance);

#ifdef __cplusplus
}
#endif

#endif /* IR_TIMING_H */
