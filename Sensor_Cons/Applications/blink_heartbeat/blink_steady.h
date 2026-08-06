/**
 * @file blink_steady.h
 * @brief Behaviour: the auxiliary external LED blinks on and off at a fixed rate.
 */

#ifndef BLINK_STEADY_H
#define BLINK_STEADY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Set up the LED and start blinking
 * @retval true on success
 */
bool BlinkSteady_Init(void);

/**
 * @brief  Advance the blink; call from the super-loop
 * @param  nowMs Current tick in milliseconds
 */
void BlinkSteady_Task(uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* BLINK_STEADY_H */
