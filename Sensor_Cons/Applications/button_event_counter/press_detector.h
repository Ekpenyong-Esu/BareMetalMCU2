/**
 * @file    press_detector.h
 * @brief   Press / long-press detection.
 *
 * Single responsibility: turn a debounced button into press and long-press
 * events. It knows nothing about counting or LEDs.
 */

#ifndef PRESS_DETECTOR_H
#define PRESS_DETECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "button.h"
#include <stdbool.h>
#include <stdint.h>

/* Hold the button this long for a long-press (ms). */
#define PRESS_LONG_MS   1000u

/* Handle -------------------------------------------------------------------*/
typedef struct {
    ButtonHandle_t button;      /* debounced button (driver owns the debounce) */
    uint32_t pressStartMs;      /* tick when the current press began */
    bool pressed;               /* debounced press state */
    bool longPressSent;         /* long-press already reported for this press */
    bool pressEvent;            /* pending press edge, cleared by WasPressed */
    bool longPressEvent;        /* pending long-press, cleared by WasLongPressed */
} PressDetector_t;

/* Functions ----------------------------------------------------------------*/

/**
 * @brief   Initialise the detector around a button configuration.
 * @param   handle Detector handle
 * @param   config Button configuration (port, pin, active level, debounce)
 * @retval  true if successful, false otherwise
 */
bool PressDetector_Init(PressDetector_t* handle, const ButtonConfig_t* config);

/**
 * @brief   Sample the button and update the press / long-press events.
 * @note    Call once per super-loop iteration.
 */
void PressDetector_Update(PressDetector_t* handle);

/**
 * @brief   Current debounced press state.
 */
bool PressDetector_IsPressed(const PressDetector_t* handle);

/**
 * @brief   True once per press edge (consumes the event).
 */
bool PressDetector_WasPressed(PressDetector_t* handle);

/**
 * @brief   True once per long-press (consumes the event).
 */
bool PressDetector_WasLongPressed(PressDetector_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* PRESS_DETECTOR_H */
