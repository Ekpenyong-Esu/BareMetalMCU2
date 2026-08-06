/**
 * @file heartbeat_rhythm.c
 * @brief The lub-dub beat timings shared by every heartbeat behaviour.
 */

/* Includes ------------------------------------------------------------------*/
#include "heartbeat_rhythm.h"

#include "led_pwm.h"

/* Private constants ---------------------------------------------------------*/
#define RHYTHM_FRAME_COUNT   4u
#define BEAT_DECAY_MS      250u

/** Two quick beats followed by a long rest, like a resting heart. */
static const LedPatternFrame_t s_rhythmFrames[RHYTHM_FRAME_COUNT] = {
    {LED_ON,   70u},   /* lub  */
    {LED_OFF, 150u},
    {LED_ON,   70u},   /* dub  */
    {LED_OFF, 900u}    /* rest */
};

/* Exported functions --------------------------------------------------------*/

bool HeartbeatRhythm_Start(LedPattern_t* pattern)
{
    if (!LedPattern_Init(pattern, s_rhythmFrames, RHYTHM_FRAME_COUNT)) {
        return false;
    }

    LedPattern_Start(pattern, 0u);
    return true;
}

uint8_t HeartbeatRhythm_Brightness(const LedPattern_t* pattern, uint32_t nowMs)
{
    if (LedPattern_GetState(pattern) == LED_ON) {
        return LED_PWM_MAX_BRIGHTNESS;
    }

    uint32_t elapsedMs = LedPattern_GetElapsedMs(pattern, nowMs);

    if (elapsedMs >= BEAT_DECAY_MS) {
        return 0u;
    }

    uint32_t level = (LED_PWM_MAX_BRIGHTNESS * (BEAT_DECAY_MS - elapsedMs)) /
                     BEAT_DECAY_MS;

    /* Square for perceptual linearity */
    return (uint8_t)((level * level) / LED_PWM_MAX_BRIGHTNESS);
}
