/**
 * @file    press_counter.h
 * @brief   Press counter.
 *
 * Single responsibility: count presses. It knows nothing about buttons or LEDs.
 */

#ifndef PRESS_COUNTER_H
#define PRESS_COUNTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Highest count the display can show (2 count LEDs -> 0..3). */
#define PRESS_COUNTER_MAX   3u

/* Handle -------------------------------------------------------------------*/
typedef struct {
    uint32_t count;             /* current press count */
} PressCounter_t;

/* Functions ----------------------------------------------------------------*/

/**
 * @brief   Initialise the counter to zero.
 * @retval  true if successful, false otherwise
 */
bool PressCounter_Init(PressCounter_t* handle);

/**
 * @brief   Increment the count, saturating at PRESS_COUNTER_MAX.
 */
void PressCounter_Increment(PressCounter_t* handle);

/**
 * @brief   Reset the count to zero.
 */
void PressCounter_Reset(PressCounter_t* handle);

/**
 * @brief   Current press count.
 */
uint32_t PressCounter_GetCount(const PressCounter_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* PRESS_COUNTER_H */
