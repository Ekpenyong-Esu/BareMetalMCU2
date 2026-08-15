/**
 * @file    press_detector.c
 * @brief   Press / long-press detection.
 *
 * Single responsibility: sample the debounced button and report press and
 * long-press events. The BUTTON driver owns the debounce; this module only
 * watches the debounced state and measures how long it is held.
 */

#include "press_detector.h"

#include "stm32f4xx_hal.h"

bool PressDetector_Init(PressDetector_t* handle, const ButtonConfig_t* config)
{
    if (handle == NULL || config == NULL) {
        return false;
    }

    handle->pressStartMs = 0u;
    handle->pressed = false;
    handle->longPressSent = false;
    handle->pressEvent = false;
    handle->longPressEvent = false;

    return Button_InitCustom(&handle->button, config);
}

void PressDetector_Update(PressDetector_t* handle)
{
    if (handle == NULL) {
        return;
    }

    const bool pressed = Button_IsPressed(&handle->button);
    const uint32_t nowMs = HAL_GetTick();

    if (pressed && !handle->pressed) {
        /* Rising edge: a new press began. */
        handle->pressed = true;
        handle->pressStartMs = nowMs;
        handle->longPressSent = false;
        handle->pressEvent = true;
    } else if (!pressed && handle->pressed) {
        /* Falling edge: the press ended. */
        handle->pressed = false;
    } else if (pressed && !handle->longPressSent &&
               (nowMs - handle->pressStartMs) >= PRESS_LONG_MS) {
        /* Held long enough: report the long-press once. */
        handle->longPressEvent = true;
        handle->longPressSent = true;
    }
}

bool PressDetector_IsPressed(const PressDetector_t* handle)
{
    if (handle == NULL) {
        return false;
    }

    return handle->pressed;
}

bool PressDetector_WasPressed(PressDetector_t* handle)
{
    if (handle == NULL) {
        return false;
    }

    const bool event = handle->pressEvent;
    handle->pressEvent = false;
    return event;
}

bool PressDetector_WasLongPressed(PressDetector_t* handle)
{
    if (handle == NULL) {
        return false;
    }

    const bool event = handle->longPressEvent;
    handle->longPressEvent = false;
    return event;
}
