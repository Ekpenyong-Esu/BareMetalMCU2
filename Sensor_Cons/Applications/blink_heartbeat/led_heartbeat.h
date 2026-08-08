/**
 * @file led_heartbeat.h
 * @brief Behaviour: lub-dub heartbeat on the red on-board LED, each beat fully
 *        lit or fully dark.
 */

#ifndef LED_HEARTBEAT_H
#define LED_HEARTBEAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Set up the LED and start the rhythm
 * @retval true on success
 */
bool LedHeartbeat_Init(void);

/**
 * @brief  Advance the rhythm; call from the super-loop
 * @param  nowMs Current tick in milliseconds
 */
void LedHeartbeat_Task(uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* LED_HEARTBEAT_H */
