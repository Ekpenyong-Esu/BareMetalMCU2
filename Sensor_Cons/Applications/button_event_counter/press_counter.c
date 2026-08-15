/**
 * @file    press_counter.c
 * @brief   Press counter.
 *
 * Single responsibility: keep a press count. No button or LED knowledge here.
 */

#include "press_counter.h"

bool PressCounter_Init(PressCounter_t* handle)
{
    if (handle == NULL) {
        return false;
    }

    handle->count = 0u;
    return true;
}

void PressCounter_Increment(PressCounter_t* handle)
{
    if (handle == NULL) {
        return;
    }

    if (handle->count < PRESS_COUNTER_MAX) {
        handle->count++;
    }
}

void PressCounter_Reset(PressCounter_t* handle)
{
    if (handle == NULL) {
        return;
    }

    handle->count = 0u;
}

uint32_t PressCounter_GetCount(const PressCounter_t* handle)
{
    return (handle != NULL) ? handle->count : 0u;
}
