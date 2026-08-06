/**
 * @file heartbeat_fade_software.h
 * @brief Behaviour: lub-dub heartbeat on the green on-board LED, each beat
 *        fading in and out. The CPU generates the PWM.
 */

#ifndef HEARTBEAT_FADE_SOFTWARE_H
#define HEARTBEAT_FADE_SOFTWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Set up the LED, the software PWM channel and the rhythm
 * @retval true on success
 */
bool HeartbeatFadeSoftware_Init(void);

/**
 * @brief  Advance the rhythm and service the software PWM
 * @note   Call as often as possible; the PWM resolution equals the interval
 *         between two calls, so any blocking work shows up as flicker.
 * @param  nowMs Current tick in milliseconds
 */
void HeartbeatFadeSoftware_Task(uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* HEARTBEAT_FADE_SOFTWARE_H */
