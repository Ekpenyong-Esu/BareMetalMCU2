/**
 * @file heartbeat_fade_software.c
 * @brief Behaviour: lub-dub heartbeat on the green on-board LED, each beat
 *        fading in and out. The CPU generates the PWM.
 * @details Port G has no timer alternate function, so the on-board LEDs can
 *          only be dimmed by toggling the pin in the super-loop. The result is
 *          a soft beat instead of a hard one - at the cost of being sensitive
 *          to anything that stalls the loop.
 */

/* Includes ------------------------------------------------------------------*/
#include "heartbeat_fade_software.h"

#include "board.h"
#include "heartbeat_rhythm.h"
#include "led_pwm.h"
#include "sys.h"

/* Private variables ---------------------------------------------------------*/
static LedHandle_t  s_led;
static LedPwm_t     s_pwm;
static LedPattern_t s_pattern;

/* Exported functions --------------------------------------------------------*/

bool HeartbeatFadeSoftware_Init(void)
{
    const LedConfig_t config = {
        .port = BOARD_LED_GREEN_PORT,
        .pin = BOARD_LED_GREEN_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };

    if (!Led_InitCustom(&s_led, &config)) {
        return false;
    }

    if (!LedPwm_Init(&s_pwm, &s_led, LED_PWM_DEFAULT_PERIOD_US) ||
        !LedPwm_Start(&s_pwm, SYS_GetMicros())) {
        return false;
    }

    return HeartbeatRhythm_Start(&s_pattern);
}

void HeartbeatFadeSoftware_Task(uint32_t nowMs)
{
    LedPattern_Update(&s_pattern, nowMs);
    LedPwm_SetBrightness(&s_pwm, HeartbeatRhythm_Brightness(&s_pattern, nowMs));
    LedPwm_Update(&s_pwm, SYS_GetMicros());
}
