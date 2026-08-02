/**
 * @file led_blink.c
 * @brief Fixed-rate blink engine implementation.
 */

/* Includes ------------------------------------------------------------------*/
#include "led_blink.h"

/* Exported functions --------------------------------------------------------*/

bool LedBlink_Init(LedBlink_t* blink, LedHandle_t* led)
{
    if (blink == NULL || led == NULL) {
        return false;
    }

    blink->led = led;
    blink->periodMs = 0u;
    blink->lastToggleMs = 0u;
    blink->running = false;
    return true;
}

bool LedBlink_Start(LedBlink_t* blink, uint32_t periodMs, uint32_t nowMs)
{
    if (blink == NULL || blink->led == NULL || periodMs == 0u) {
        return false;
    }

    blink->periodMs = periodMs;
    blink->lastToggleMs = nowMs;
    blink->running = true;
    return true;
}

bool LedBlink_Stop(LedBlink_t* blink)
{
    if (blink == NULL || blink->led == NULL) {
        return false;
    }

    blink->running = false;
    return Led_Off(blink->led);
}

bool LedBlink_Update(LedBlink_t* blink, uint32_t nowMs)
{
    if (blink == NULL || blink->led == NULL) {
        return false;
    }

    if (!blink->running) {
        return true;
    }

    uint32_t halfPeriodMs = blink->periodMs / 2u;

    if ((nowMs - blink->lastToggleMs) >= halfPeriodMs) {
        Led_Toggle(blink->led);
        blink->lastToggleMs = nowMs;
    }

    return true;
}

bool LedBlink_IsRunning(const LedBlink_t* blink)
{
    return (blink != NULL) && blink->running;
}
