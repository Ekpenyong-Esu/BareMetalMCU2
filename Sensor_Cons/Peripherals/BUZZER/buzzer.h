/**
 ******************************************************************************
 * @file    buzzer.h
 * @brief   Buzzer driver — make sounds with a buzzer
 * @details Supports two kinds of buzzers:
 *          - Active buzzer: has its own oscillator, just needs on/off (GPIO).
 *          - Passive buzzer: needs the MCU to generate the tone (PWM).
 *
 * How it works (in simple words):
 * - Active: set the GPIO high to beep, low to stop.
 * - Passive: the timer makes a square wave; changing the frequency changes
 *   the pitch, changing the duty changes the volume.
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "stm32f4xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Buzzer operating mode.
 *
 * Active buzzers contain an internal oscillator; the MCU only gates the
 * power with a GPIO pin. Passive buzzers are simple electromagnetic or
 * piezo transducers that require the MCU to generate the audio-frequency
 * square wave via PWM.
 */
typedef enum {
    BUZZER_MODE_ACTIVE_GPIO = 0, ///< Active buzzer driven by GPIO high/low
    BUZZER_MODE_PASSIVE_PWM      ///< Passive buzzer driven by PWM
} BuzzerMode_t;

/**
 * @brief Buzzer instance descriptor.
 *
 * Only the fields relevant to the selected mode are used:
 *  - Active mode:  port, pin
 *  - Passive mode: htim, channel, pwmTimerClockHz
 */
typedef struct {
    BuzzerMode_t mode;        ///< Active or passive
    GPIO_TypeDef *port;       ///< GPIO port (active mode only)
    uint16_t pin;             ///< GPIO pin (active mode only)
    TIM_HandleTypeDef *htim;  ///< Timer handle (passive mode only)
    uint32_t channel;         ///< Timer channel (passive mode only)
    uint32_t pwmTimerClockHz; ///< Timer input clock in Hz (passive mode only)
} Buzzer_t;

/**
 * @brief Initialize an active buzzer on a GPIO output pin.
 *
 * Configures the pin as push-pull output, no pull, low speed, and drives it
 * low (silent). The buzzer's internal oscillator determines the pitch; the
 * MCU only controls on/off.
 *
 * @param buzzer  Buzzer instance to initialize.
 * @param port    GPIO port (e.g., GPIOE).
 * @param pin     GPIO pin (e.g., GPIO_PIN_6).
 * @return HAL_OK on success, HAL_ERROR on invalid arguments or GPIO init failure.
 */
HAL_StatusTypeDef Buzzer_InitActive(Buzzer_t *buzzer, GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Initialize a passive buzzer on a timer PWM channel.
 *
 * The timer must already be configured for PWM output on the given channel.
 * The timer's input clock frequency (after any APB prescaler) is recorded so
 * that Buzzer_Tone() can compute prescaler/ARR values for arbitrary audio
 * frequencies.
 *
 * @param buzzer          Buzzer instance to initialize.
 * @param htim            Timer handle (e.g., &htim9).
 * @param channel         Timer channel (e.g., TIM_CHANNEL_1).
 * @param timerClockHz    Timer input clock in Hz (e.g., 84000000 for TIM9 on F4).
 * @return HAL_OK on success, HAL_ERROR on invalid arguments or PWM start failure.
 */
HAL_StatusTypeDef Buzzer_InitPassive(Buzzer_t *buzzer, TIM_HandleTypeDef *htim, uint32_t channel,
                                     uint32_t timerClockHz);

/**
 * @brief Turn the buzzer on.
 *
 * Active mode: drives the GPIO pin high.
 * Passive mode: sets the PWM compare to 50% duty (square wave) at the
 * frequency previously programmed by Buzzer_Tone(). If Buzzer_Tone() has
 * not been called yet, the frequency is whatever the timer was left at.
 *
 * @param buzzer  Buzzer instance.
 */
void Buzzer_On(Buzzer_t *buzzer);

/**
 * @brief Turn the buzzer off (silence).
 *
 * Active mode: drives the GPIO pin low.
 * Passive mode: sets the PWM compare to 0 (output held low).
 *
 * @param buzzer  Buzzer instance.
 */
void Buzzer_Off(Buzzer_t *buzzer);

/**
 * @brief Set the output frequency and duty cycle (passive mode only).
 *
 * Computes the timer prescaler and auto-reload register (ARR) to generate
 * the requested frequency with the requested duty cycle. The STM32F4 timers
 * have a 16-bit ARR, so a prescaler is automatically chosen to keep the
 * period within 65535. An update event (UG) is generated to load the
 * shadow registers immediately.
 *
 * @param buzzer       Buzzer instance (must be in passive mode).
 * @param frequencyHz  Desired frequency in Hz (1..timerClockHz/2).
 * @param dutyPercent  Duty cycle 0..100 (50 gives a square wave).
 * @return HAL_OK on success, HAL_ERROR if buzzer is not passive, frequency
 *         is out of range, or dutyPercent > 100.
 */
HAL_StatusTypeDef Buzzer_Tone(Buzzer_t *buzzer, uint32_t frequencyHz, uint8_t dutyPercent);

/**
 * @brief Check whether the buzzer is currently producing sound.
 *
 * Active mode: reads the GPIO output data register.
 * Passive mode: checks whether the PWM compare register is non-zero.
 *
 * @param buzzer  Buzzer instance.
 * @return true if on, false if off or buzzer is NULL.
 */
bool Buzzer_IsOn(const Buzzer_t *buzzer);

#ifdef __cplusplus
}
#endif

#endif /* BUZZER_H */
