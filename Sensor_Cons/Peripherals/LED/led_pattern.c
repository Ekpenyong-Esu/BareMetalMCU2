/**
 * @file led_pattern.c
 * @brief Table-driven LED pattern player implementation.
 */

/* Includes ------------------------------------------------------------------*/
#include "led_pattern.h"

/* Exported functions --------------------------------------------------------*/

bool LedPattern_Init(LedPattern_t *pattern, const LedPatternFrame_t *frames, uint32_t frameCount) {
    if (pattern == NULL || frames == NULL || frameCount == 0u) {
        return false;
    }

    pattern->frames = frames;
    pattern->frameCount = frameCount;
    pattern->currentFrame = 0u;
    pattern->frameStartMs = 0u;
    return true;
}

void LedPattern_Start(LedPattern_t *pattern, uint32_t nowMs) {
    if (pattern == NULL) {
        return;
    }

    pattern->currentFrame = 0u;
    pattern->frameStartMs = nowMs;
}

void LedPattern_Update(LedPattern_t *pattern, uint32_t nowMs) {
    if (pattern == NULL) {
        return;
    }

    uint32_t elapsedMs = nowMs - pattern->frameStartMs;

    if (elapsedMs >= pattern->frames[pattern->currentFrame].durationMs) {
        pattern->currentFrame = (pattern->currentFrame + 1u) % pattern->frameCount;
        pattern->frameStartMs = nowMs;
    }
}

LedState_t LedPattern_GetState(const LedPattern_t *pattern) {
    if (pattern == NULL) {
        return LED_OFF;
    }

    return pattern->frames[pattern->currentFrame].state;
}

uint32_t LedPattern_GetElapsedMs(const LedPattern_t *pattern, uint32_t nowMs) {
    if (pattern == NULL) {
        return 0u;
    }

    return nowMs - pattern->frameStartMs;
}
