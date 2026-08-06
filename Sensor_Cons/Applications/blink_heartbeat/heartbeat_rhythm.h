/**
 * @file heartbeat_rhythm.h
 * @brief The lub-dub beat timings shared by every heartbeat behaviour.
 * @details Kept in one place so the on/off, software-fade and hardware-fade
 *          behaviours beat in the same rhythm and only ever differ in how they
 *          drive their LED.
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
 * @details Every behaviour starts from tick 0, so the three hearts beat together
 *          however late in the boot their own Init runs.
 * @param  pattern Player owned by the calling behaviour
 * @retval true on success
 */
bool HeartbeatRhythm_Start(LedPattern_t* pattern);

/**
 * @brief  Turn the current beat into a brightness level for the fading variants
 * @details A beat snaps to full and then decays through the rest that follows
 *          it. Ramping within the 70 ms beat itself would be invisible - the
 *          eye integrates anything that short into a plain flash.
 * @param  pattern Player started by HeartbeatRhythm_Start
 * @param  nowMs   Current tick in milliseconds
 * @retval Brightness in the range 0..LED_PWM_MAX_BRIGHTNESS
 */
uint8_t HeartbeatRhythm_Brightness(const LedPattern_t* pattern, uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* HEARTBEAT_RHYTHM_H */
