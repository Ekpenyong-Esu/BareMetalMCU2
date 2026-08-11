/**
 * @file led_software_pwm.c
 * @brief Software PWM implementation for a GPIO-connected LED.
 */

/* Includes ------------------------------------------------------------------*/
#include "led_software_pwm.h"

/* Exported functions --------------------------------------------------------*/

bool LedPwm_Init(LedPwm_t* pwm, LedHandle_t* led, uint32_t periodUs)
{
    if (pwm == NULL || led == NULL || periodUs == 0u) {
        return false;
    }

    pwm->led = led;
    pwm->periodUs = periodUs;
    pwm->cycleStartUs = 0u;
    pwm->brightness = 0u;
    pwm->running = false;
    return true;
}

bool LedPwm_Start(LedPwm_t* pwm, uint32_t nowUs)
{
    if (pwm == NULL || pwm->led == NULL) {
        return false;
    }

    pwm->cycleStartUs = nowUs;
    pwm->running = true;
    return true;
}

bool LedPwm_Stop(LedPwm_t* pwm)
{
    if (pwm == NULL || pwm->led == NULL) {
        return false;
    }

    pwm->running = false;
    return Led_Off(pwm->led);
}

bool LedPwm_SetBrightness(LedPwm_t* pwm, uint8_t brightness)
{
    if (pwm == NULL) {
        return false;
    }

    pwm->brightness = (brightness > LED_PWM_MAX_BRIGHTNESS) ? LED_PWM_MAX_BRIGHTNESS
                                                            : brightness;
    return true;
}

uint8_t LedPwm_GetBrightness(const LedPwm_t* pwm)
{
    return (pwm != NULL) ? pwm->brightness : 0u;
}

bool LedPwm_Update(LedPwm_t* pwm, uint32_t nowUs)
{
    if (pwm == NULL || pwm->led == NULL) {
        return false;
    }

    if (!pwm->running) {
        return true;
    }

    uint32_t elapsedUs = nowUs - pwm->cycleStartUs;

    if (elapsedUs >= pwm->periodUs) {
        pwm->cycleStartUs = nowUs;
        elapsedUs = 0u;
    }

    /* Multiply before dividing: periodUs is not necessarily a multiple of 100. */
    uint32_t onTimeUs = (pwm->periodUs * pwm->brightness) / LED_PWM_MAX_BRIGHTNESS;
    LedState_t wanted = (elapsedUs < onTimeUs) ? LED_ON : LED_OFF;

    if (Led_GetState(pwm->led) != wanted) {
        Led_SetState(pwm->led, wanted);
    }

    return true;
}

uint8_t LedPwm_Waveform_Smooth(uint32_t elapsedMs, uint32_t periodMs)
{
    uint32_t halfPeriodMs = periodMs / 2u;
    uint32_t position = elapsedMs % periodMs;

    /* Triangle ramp 0..LED_PWM_MAX_BRIGHTNESS..0 */
    uint32_t level = (position < halfPeriodMs)
                   ? (position * LED_PWM_MAX_BRIGHTNESS) / halfPeriodMs
                   : ((periodMs - position) * LED_PWM_MAX_BRIGHTNESS) / halfPeriodMs;

    /* Square for perceptual linearity */
    return (uint8_t)((level * level) / LED_PWM_MAX_BRIGHTNESS);
}
