/**
 * @file led_pwm.h
 * @brief Software PWM (brightness control) for a GPIO-connected LED.
 * @details The on-board LEDs of the STM32F429I-DISC1 (PG13/PG14) have no timer
 *          alternate function, so hardware PWM is not available on them. This
 *          module bit-bangs PWM cooperatively: call LedPwm_Update() as often as
 *          possible from the super-loop and it decides whether the LED should be
 *          on or off inside the current PWM cycle.
 *
 *          The caller supplies the current time in microseconds, so this module
 *          owns duty-cycle handling only - not the time source.
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* Exported constants --------------------------------------------------------*/
#define LED_PWM_MAX_BRIGHTNESS      100u    /**< Brightness is a percentage */
#define LED_PWM_DEFAULT_PERIOD_US   5000u   /**< 200 Hz: above the flicker threshold */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Software PWM state
 */
typedef struct {
    LedHandle_t* led;       /**< LED being driven */
    uint32_t periodUs;      /**< PWM period in microseconds */
    uint32_t cycleStartUs;  /**< Start of the current PWM cycle */
    uint8_t brightness;     /**< Duty cycle, 0..LED_PWM_MAX_BRIGHTNESS */
    bool running;           /**< PWM active flag */
} LedPwm_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Bind a software PWM channel to an initialized LED
 * @param   pwm Pointer to PWM state
 * @param   led Pointer to an initialized LED handle
 * @param   periodUs PWM period in microseconds (must be non-zero)
 * @retval  true if successful, false otherwise
 */
bool LedPwm_Init(LedPwm_t* pwm, LedHandle_t* led, uint32_t periodUs);

/**
 * @brief   Start modulating the LED
 * @param   pwm Pointer to PWM state
 * @param   nowUs Current time in microseconds
 * @retval  true if successful, false otherwise
 */
bool LedPwm_Start(LedPwm_t* pwm, uint32_t nowUs);

/**
 * @brief   Stop modulating and switch the LED off
 * @param   pwm Pointer to PWM state
 * @retval  true if successful, false otherwise
 */
bool LedPwm_Stop(LedPwm_t* pwm);

/**
 * @brief   Set the duty cycle
 * @param   pwm Pointer to PWM state
 * @param   brightness 0 (off) to LED_PWM_MAX_BRIGHTNESS (full on), clamped
 * @retval  true if successful, false otherwise
 */
bool LedPwm_SetBrightness(LedPwm_t* pwm, uint8_t brightness);

/**
 * @brief   Get the current duty cycle
 * @param   pwm Pointer to PWM state
 * @retval  Brightness in the range 0..LED_PWM_MAX_BRIGHTNESS
 */
uint8_t LedPwm_GetBrightness(const LedPwm_t* pwm);

/**
 * @brief   Drive the LED for the current point in the PWM cycle
 * @param   pwm Pointer to PWM state
 * @param   nowUs Current time in microseconds
 * @note    Call as often as possible; the PWM resolution equals the interval
 *          between two calls.
 * @retval  true if successful, false otherwise
 */
bool LedPwm_Update(LedPwm_t* pwm, uint32_t nowUs);

/**
 * @brief   Triangle-wave brightness curve corrected for perceptual linearity
 * @details Produces a 0..LED_PWM_MAX_BRIGHTNESS value that ramps up then down,
 *          squared so brightness changes appear uniform to the human eye.
 *          Suitable for breathing / fading effects without table storage.
 * @param   elapsedMs Time elapsed since the waveform started
 * @param   periodMs  Duration of one complete ramp-up + ramp-down
 * @retval  Brightness in the range 0..LED_PWM_MAX_BRIGHTNESS
 */
uint8_t LedPwm_Waveform_Smooth(uint32_t elapsedMs, uint32_t periodMs);

#ifdef __cplusplus
}
#endif

#endif /* LED_PWM_H */
