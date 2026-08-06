/**
 * @file heartbeat_fade_hardware.h
 * @brief Behaviour: lub-dub heartbeat on the external LED, each beat fading in
 *        and out. A timer generates the PWM.
 */

#ifndef HEARTBEAT_FADE_HARDWARE_H
#define HEARTBEAT_FADE_HARDWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Set up the timer PWM channel and the rhythm
 * @retval true on success
 */
bool HeartbeatFadeHardware_Init(void);

/**
 * @brief  Advance the rhythm and hand the new brightness to the timer
 * @param  nowMs Current tick in milliseconds
 */
void HeartbeatFadeHardware_Task(uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* HEARTBEAT_FADE_HARDWARE_H */
