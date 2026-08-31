/**
 * @file led_pwm_timer.h
 * @brief LED brightness driven by a hardware timer PWM channel.
 * @details Owns one responsibility: turning a brightness percentage into a
 *          compare value on a timer output. Once started the timer generates
 *          the waveform on its own, so there is nothing to poll.
 *
 *          This is the hardware counterpart of led_software_pwm.h, which bit-bangs the
 *          same brightness on pins that have no timer alternate function.
 *          Both take brightness in 0..LED_PWM_MAX_BRIGHTNESS, so a caller can
 *          swap one for the other without changing its own logic.
 *
 * Example
 * -------
 *   LedPwmTimer_t pwm;
 *   const LedPwmTimerConfig_t config = {
 *       .port = GPIOA, .pin = GPIO_PIN_5, .alternate = GPIO_AF1_TIM2,
 *       .timer = TIM2, .channel = TIM_CHANNEL_1, .pwmFrequencyHz = 1000u
 *   };
 *
 *   LedPwmTimer_Init(&pwm, &config);
 *   LedPwmTimer_SetBrightness(&pwm, 50u);
 */

#ifndef LED_PWM_TIMER_H
#define LED_PWM_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "led_pwm_types.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
#define LED_PWM_TIMER_STEPS   1024u  /**< Duty resolution of one PWM period */

/* Exported types ------------------------------------------------------------*/

/** Which pin and timer channel to drive, and at what carrier frequency. */
typedef struct {
    GPIO_TypeDef* port;            /**< Port of the PWM output pin */
    uint16_t      pin;             /**< PWM output pin */
    uint8_t       alternate;       /**< GPIO alternate function selecting the timer */
    TIM_TypeDef*  timer;           /**< Timer peripheral; must have output channels */
    uint32_t      channel;         /**< TIM_CHANNEL_1..4 */
    uint32_t      pwmFrequencyHz;  /**< Carrier frequency; keep above ~100 Hz to avoid flicker */
    bool          activeLow;       /**< True when the LED lights on a low output */
} LedPwmTimerConfig_t;

typedef struct {
    TIM_HandleTypeDef htim;     /**< Timer handle owned by this module */
    uint32_t          channel;  /**< Channel being modulated */
} LedPwmTimer_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Enable the timer clock, route the pin, and start the output fully off
 * @param  pwm    Channel handle
 * @param  config Pin, timer and carrier description
 * @retval true on success, false if the timer has no output channels or the
 *         requested frequency is unreachable from the timer clock
 */
bool LedPwmTimer_Init(LedPwmTimer_t* pwm, const LedPwmTimerConfig_t* config);

/**
 * @brief  Set the duty cycle
 * @param  pwm        Channel handle
 * @param  brightness 0 (off) to LED_PWM_MAX_BRIGHTNESS (full on), clamped
 * @retval true on success
 */
bool LedPwmTimer_SetBrightness(LedPwmTimer_t* pwm, uint8_t brightness);

#ifdef __cplusplus
}
#endif

#endif /* LED_PWM_TIMER_H */
