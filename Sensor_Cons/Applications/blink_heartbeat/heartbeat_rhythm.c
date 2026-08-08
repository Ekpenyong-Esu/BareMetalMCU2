/**
 * @file heartbeat_rhythm.c
 * @brief The lub-dub beat timings shared by every heartbeat behaviour.
 */

/* Includes ------------------------------------------------------------------*/
#include "heartbeat_rhythm.h"

/* Private constants ---------------------------------------------------------*/
#define RHYTHM_FRAME_COUNT   4u
#define BEAT_PULSE_MS      220u

/** Two quick beats followed by a long rest, like a resting heart. */
static const LedPatternFrame_t s_rhythmFrames[RHYTHM_FRAME_COUNT] = {
    {LED_ON,  BEAT_PULSE_MS},   /* lub  */
    {LED_OFF, 150u},
    {LED_ON,  BEAT_PULSE_MS},   /* dub  */
    {LED_OFF, 600u}             /* rest */
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
