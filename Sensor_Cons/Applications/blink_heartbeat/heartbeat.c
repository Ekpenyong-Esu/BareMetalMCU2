/**
 * @file heartbeat.c
 * @brief Generic table-driven heartbeat-pattern engine implementation.
 */

/* Includes ------------------------------------------------------------------*/
#include "heartbeat.h"

/* Exported functions --------------------------------------------------------*/

bool Heartbeat_Init(Heartbeat_t* hb, LedHandle_t* led,
                    const HeartbeatFrame_t* pattern, uint32_t frameCount)
{
    if (hb == NULL || led == NULL || pattern == NULL || frameCount == 0u) {
        return false;
    }

    hb->led = led;
    hb->pattern = pattern;
    hb->frameCount = frameCount;
    hb->currentFrame = 0u;
    hb->frameStartMs = 0u;
    return true;
}

void Heartbeat_Start(Heartbeat_t* hb, uint32_t nowMs)
{
    if (hb == NULL || hb->led == NULL) {
        return;
    }

    hb->currentFrame = 0u;
    hb->frameStartMs = nowMs;
    Led_SetState(hb->led, hb->pattern[0u].state);
}

void Heartbeat_Update(Heartbeat_t* hb, uint32_t nowMs)
{
    if (hb == NULL || hb->led == NULL) {
        return;
    }

    uint32_t elapsedMs = nowMs - hb->frameStartMs;
    const HeartbeatFrame_t* frame = &hb->pattern[hb->currentFrame];

    if (elapsedMs >= frame->durationMs) {
        hb->currentFrame = (hb->currentFrame + 1u) % hb->frameCount;
        hb->frameStartMs = nowMs;

        Led_SetState(hb->led, hb->pattern[hb->currentFrame].state);
    }
}
