/**
 * @file pwm_led_software.h
 * @brief Behaviour: green on-board LED breathing continuously, PWM generated
 *        by the CPU.
 */

#ifndef PWM_LED_SOFTWARE_H
#define PWM_LED_SOFTWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Set up the LED and the software PWM channel
 * @retval true on success
 */
bool PwmLedSoftware_Init(void);

/**
 * @brief  Set the brightness for this instant and service the software PWM
 * @note   Call as often as possible; the PWM resolution equals the interval
 *         between two calls, so any blocking work shows up as flicker.
 * @param  nowMs Current tick in milliseconds
 */
void PwmLedSoftware_Task(uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* PWM_LED_SOFTWARE_H */
