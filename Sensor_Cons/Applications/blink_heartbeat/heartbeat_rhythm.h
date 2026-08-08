/**
 * @file heartbeat_rhythm.h
 * @brief The lub-dub beat timings shared by every heartbeat behaviour.
 * @details Kept separate from the behaviour so the beat can be reused by any
 *          module that wants to drive an LED, or a buzzer, in that rhythm.
 */

#ifndef HEARTBEAT_RHYTHM_H
#define HEARTBEAT_RHYTHM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "led_pattern.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Bind a player to the lub-dub table and start it
 * @details Every behaviour starts from tick 0, so the hearts beat together
 *          however late in the boot their own Init runs.
 * @param  pattern Player owned by the calling behaviour
 * @retval true on success
 */
bool HeartbeatRhythm_Start(LedPattern_t* pattern);

#ifdef __cplusplus
}
#endif

#endif /* HEARTBEAT_RHYTHM_H */
