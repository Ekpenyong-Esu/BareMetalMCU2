/**
 * @file heartbeat_onoff.h
 * @brief Behaviour: lub-dub heartbeat on the red on-board LED, each beat fully
 *        lit or fully dark.
 */

#ifndef HEARTBEAT_ONOFF_H
#define HEARTBEAT_ONOFF_H

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
bool HeartbeatOnOff_Init(void);

/**
 * @brief  Advance the rhythm; call from the super-loop
 * @param  nowMs Current tick in milliseconds
 */
void HeartbeatOnOff_Task(uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* HEARTBEAT_ONOFF_H */
