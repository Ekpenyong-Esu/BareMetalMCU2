/**
 * @file led_pwm_timer.c
 * @brief Hardware timer PWM brightness implementation.
 */

/* Includes ------------------------------------------------------------------*/
#include "led_pwm_timer.h"

#include "gpio.h"
#include "tim_pwm.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Route the pin to the timer's output compare unit
 * @param   config Pin description
 */
static bool LedPwmTimer_PinInit(const LedPwmTimerConfig_t* config)
{
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = config->pin;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Alternate = config->alternate;

    return GPIO_Driver_Pin_Init(config->port, &gpioInit) == HAL_OK;
}

/* Exported functions --------------------------------------------------------*/

bool LedPwmTimer_Init(LedPwmTimer_t* pwm, const LedPwmTimerConfig_t* config)
{
    if (pwm == NULL || config == NULL || config->port == NULL) {
        return false;
    }

    if (TIM_PWM_InitHz(&pwm->htim, config->timer, config->pwmFrequencyHz,
                       LED_PWM_TIMER_STEPS) != HAL_OK) {  // Initialize the timer for PWM output with the specified frequency and resolution
        return false;
    }

    if (!LedPwmTimer_PinInit(config)) {  // Initialize the GPIO pin for the timer's output compare function
        return false;
    }

    pwm->channel = config->channel;

    return ((TIM_PWM_ConfigChannel(&pwm->htim, pwm->channel, 0u) == HAL_OK) &&  // initialize the timer channel with a duty cycle of 0 (LED off) and start the PWM output
           (TIM_PWM_Start(&pwm->htim, pwm->channel) == HAL_OK)) != 0;
}

bool LedPwmTimer_SetBrightness(LedPwmTimer_t* pwm, uint8_t brightness)
{
    if (pwm == NULL) {
        return false;
    }

    if (brightness > LED_PWM_MAX_BRIGHTNESS) {
        brightness = LED_PWM_MAX_BRIGHTNESS;
    }

    TIM_PWM_SetDuty(&pwm->htim, pwm->channel,
                    (brightness * LED_PWM_TIMER_STEPS) / LED_PWM_MAX_BRIGHTNESS);
    return true;
}
