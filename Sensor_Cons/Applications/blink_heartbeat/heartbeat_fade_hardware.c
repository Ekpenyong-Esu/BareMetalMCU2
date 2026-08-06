/**
 * @file heartbeat_fade_hardware.c
 * @brief Behaviour: lub-dub heartbeat on the external LED, each beat fading in
 *        and out. A timer generates the PWM.
 * @details Same rhythm and same ramp as heartbeat_fade_software.c, but TIM2
 *          generates the waveform in hardware. The task only writes a compare
 *          value, so the beat stays perfectly steady no matter what the loop
 *          is doing.
 */

/* Includes ------------------------------------------------------------------*/
#include "heartbeat_fade_hardware.h"

#include "board.h"
#include "heartbeat_rhythm.h"
#include "led_pwm_timer.h"

/* Private constants ---------------------------------------------------------*/
#define PWM_FREQUENCY_HZ   1000u

/* Private variables ---------------------------------------------------------*/
static LedPwmTimer_t s_pwm;
static LedPattern_t  s_pattern;

/* Exported functions --------------------------------------------------------*/

bool HeartbeatFadeHardware_Init(void)
{
    const LedPwmTimerConfig_t config = {
        .port = BOARD_LED_EXT_PORT,
        .pin = BOARD_LED_EXT_PIN,
        .alternate = BOARD_LED_EXT_AF,
        .timer = BOARD_LED_EXT_TIM,
        .channel = BOARD_LED_EXT_TIM_CH,
        .pwmFrequencyHz = PWM_FREQUENCY_HZ
    };

    if (!LedPwmTimer_Init(&s_pwm, &config)) {
        return false;
    }

    return HeartbeatRhythm_Start(&s_pattern);
}

void HeartbeatFadeHardware_Task(uint32_t nowMs)
{
    LedPattern_Update(&s_pattern, nowMs);
    LedPwmTimer_SetBrightness(&s_pwm, HeartbeatRhythm_Brightness(&s_pattern, nowMs));
}
