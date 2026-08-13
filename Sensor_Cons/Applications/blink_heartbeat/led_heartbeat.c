/**
 * @file led_heartbeat.c
 * @brief Behaviour: lub-dub heartbeat on the red on-board LED, each beat fully
 *        lit or fully dark.
 * @details The only behaviour that beats a rhythm. For dimming, which needs an
 *          uninterrupted ramp to be visible at all, see pwm_led_software.c and
 *          pwm_led_hardware.c.
 */

/* Includes ------------------------------------------------------------------*/
#include "led_heartbeat.h"

#include "board.h"
#include "heartbeat_rhythm.h"

/* Private variables ---------------------------------------------------------*/
static LedHandle_t s_led;
static LedPattern_t s_pattern;

/* Exported functions --------------------------------------------------------*/

bool LedHeartbeat_Init(void)
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

void LedHeartbeat_Task(uint32_t nowMs)
{
    LedPattern_Update(&s_pattern, nowMs);
    LedState_t state = LedPattern_GetState(&s_pattern);
    Led_SetState(&s_led, state);
}
