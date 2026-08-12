/**
 * @file led_pwm_types.h
 * @brief Shared vocabulary for the LED PWM output stages
 * @details Holds the single constant both PWM stages (software and timer)
 *          agree on: the 0..LED_PWM_MAX_BRIGHTNESS brightness scale. Putting
 *          it here lets led_software_pwm.h and led_pwm_timer.h each include
 *          it without including each other.
 */

#ifndef LED_PWM_TYPES_H
#define LED_PWM_TYPES_H

/* Exported constants --------------------------------------------------------*/
#define LED_PWM_MAX_BRIGHTNESS      100u    /**< Brightness is a percentage */

#endif /* LED_PWM_TYPES_H */
