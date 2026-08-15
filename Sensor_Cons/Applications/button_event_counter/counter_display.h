/**
 * @file    counter_display.h
 * @brief   LED display for the button event counter.
 *
 * Single responsibility: show the button state and the press count on LEDs.
 * It knows nothing about buttons or counting.
 */

#ifndef COUNTER_DISPLAY_H
#define COUNTER_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "led.h"
#include <stdbool.h>
#include <stdint.h>

/* Handle -------------------------------------------------------------------*/
typedef struct {
    LedHandle_t stateLed;       /* green: on while the button is pressed */
    LedHandle_t countLed0;      /* red:   count bit 0 */
    LedHandle_t countLed1;      /* aux:   count bit 1 */
} CounterDisplay_t;

/* Functions ----------------------------------------------------------------*/

/**
 * @brief   Initialise the three LEDs used by the display.
 * @retval  true if successful, false otherwise
 */
bool CounterDisplay_Init(CounterDisplay_t* handle);

/**
 * @brief   Update the LEDs from the current button state and count.
 * @param   handle  Display handle
 * @param   pressed True while the button is pressed
 * @param   count   Press count to show in binary on the count LEDs
 */
void CounterDisplay_Show(CounterDisplay_t* handle, bool pressed, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* COUNTER_DISPLAY_H */
