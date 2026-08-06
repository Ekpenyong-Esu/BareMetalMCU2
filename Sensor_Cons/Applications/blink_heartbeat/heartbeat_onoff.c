/**
 * @file heartbeat_onoff.c
 * @brief Behaviour: lub-dub heartbeat on the red on-board LED, each beat fully
 *        lit or fully dark.
 * @details The plainest of the three heartbeats. Compare with
 *          heartbeat_fade_software.c and heartbeat_fade_hardware.c, which beat
 *          the same rhythm with brightness.
 */

/* Includes ------------------------------------------------------------------*/
#include "heartbeat_onoff.h"

#include "board.h"
#include "heartbeat_rhythm.h"

/* Private variables ---------------------------------------------------------*/
static LedHandle_t s_led;
static LedPattern_t s_pattern;

/* Exported functions --------------------------------------------------------*/

bool HeartbeatOnOff_Init(void)
{
    const LedConfig_t config = {
        .port = BOARD_LED_RED_PORT,
        .pin = BOARD_LED_RED_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };

    if (!Led_InitCustom(&s_led, &config)) {
        return false;
    }

    return HeartbeatRhythm_Start(&s_pattern);
}

void HeartbeatOnOff_Task(uint32_t nowMs)
{
    LedPattern_Update(&s_pattern, nowMs);
    Led_SetState(&s_led, LedPattern_GetState(&s_pattern));
}
