/**
 * @file pwm_led_software.c
 * @brief Behaviour: green on-board LED breathing continuously, PWM generated
 *        by the CPU.
 * @details Port G has no timer alternate function, so the on-board LEDs can
 *          only be dimmed by toggling the pin from the super-loop. Same
 *          waveform and same period as pwm_led_hardware.c, so the two sit side
 *          by side and show what bit-banging costs: this one flickers as soon
 *          as anything stalls the loop.
 *
 *          The LED is never switched on or off by this module - only its duty
 *          cycle moves. A rhythm that forces the LED dark between pulses would
 *          read as a blink and hide the dimming entirely.
 */

/* Includes ------------------------------------------------------------------*/
#include "pwm_led_software.h"

#include "board.h"
#include "led_software_pwm.h"
#include "sys.h"

/* Private constants ---------------------------------------------------------*/
#define FADE_PERIOD_MS   4000u   /**< One full dark -> bright -> dark breath */

/* Private variables ---------------------------------------------------------*/
static LedHandle_t s_led;
static LedPwm_t    s_pwm;

/* Exported functions --------------------------------------------------------*/

bool PwmLedSoftware_Init(void)
{
    const LedConfig_t config = {
        .port = BOARD_LED_GREEN_PORT,
        .pin = BOARD_LED_GREEN_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };

    if (!Led_InitCustom(&s_led, &config)) {
        return false;
    }

    return (LedPwm_Init(&s_pwm, &s_led, LED_PWM_DEFAULT_PERIOD_US) &&
           LedPwm_Start(&s_pwm, SYS_GetMicros())) != 0;
}

void PwmLedSoftware_Task(uint32_t nowMs)
{
    /* The waveform wraps on its own period, so no start time has to be kept. */
    LedPwm_SetBrightness(&s_pwm, LedPwm_Waveform_Smooth(nowMs, FADE_PERIOD_MS));
    LedPwm_Update(&s_pwm, SYS_GetMicros());
}
