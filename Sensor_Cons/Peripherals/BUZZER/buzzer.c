/**
  ******************************************************************************
  * @file    buzzer.c
  * @brief   Buzzer driver — active (GPIO) and passive (PWM) buzzers
  * @details Active buzzers beep when powered; passive buzzers need a PWM
  *          signal to make different tones. Both start silent after init.
  */

#include "buzzer.h"
#include "gpio.h"

/**
 * @brief Initialize an active buzzer on a GPIO output pin.
 *
 * Active buzzers have a built-in oscillator. The MCU only needs to gate
 * power to the buzzer with a GPIO pin. This function configures the pin
 * as push-pull output, no pull resistor, high speed for sharp switching,
 * and drives it low (silent) initially.
 *
 * @param buzzer  Buzzer instance to initialize (must not be NULL).
 * @param port    GPIO port (e.g., GPIOE).
 * @param pin     GPIO pin (e.g., GPIO_PIN_6).
 * @return HAL_OK on success, HAL_ERROR if buzzer/port is NULL or GPIO init fails.
 */
HAL_StatusTypeDef Buzzer_InitActive(Buzzer_t *buzzer, GPIO_TypeDef *port, uint16_t pin)
{
    if (buzzer == NULL || port == NULL) {
        return HAL_ERROR;
    }
    buzzer->mode = BUZZER_MODE_ACTIVE_GPIO;
    buzzer->port = port;
    buzzer->pin = pin;
    buzzer->htim = NULL;
    buzzer->channel = 0;
    buzzer->pwmTimerClockHz = 0;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    if (GPIO_Driver_Pin_Init(port, &GPIO_InitStruct) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Start silent */
    GPIO_Driver_WritePin(port, pin, GPIO_PIN_RESET);
    return HAL_OK;
}

/**
 * @brief Initialize a passive buzzer on a timer PWM channel.
 *
 * Passive buzzers require the MCU to generate the audio waveform. This
 * function records the timer handle, channel, and the timer's input clock
 * frequency (after APB prescaler) so that Buzzer_Tone() can compute the
 * correct prescaler and ARR values for any requested frequency.
 *
 * The timer must already be configured for PWM mode on the given channel
 * (typically done by a higher layer like tone_player). This function only
 * starts the PWM output with whatever compare value the timer currently has
 * (usually 0 = silent).
 *
 * @param buzzer           Buzzer instance to initialize (must not be NULL).
 * @param htim             Timer handle (e.g., &htim9).
 * @param channel          Timer channel (e.g., TIM_CHANNEL_1).
 * @param timerClockHz     Timer input clock in Hz (e.g., 84000000 for TIM9 on F4).
 * @return HAL_OK on success, HAL_ERROR if arguments invalid or PWM start fails.
 */
HAL_StatusTypeDef Buzzer_InitPassive(Buzzer_t *buzzer, TIM_HandleTypeDef *htim,
                                     uint32_t channel, uint32_t timerClockHz)
{
    if (buzzer == NULL || htim == NULL || timerClockHz == 0) {
        return HAL_ERROR;
    }
    buzzer->mode = BUZZER_MODE_PASSIVE_PWM;
    buzzer->htim = htim;
    buzzer->channel = channel;
    buzzer->pwmTimerClockHz = timerClockHz;
    buzzer->port = NULL;
    buzzer->pin = 0;

    return HAL_TIM_PWM_Start(htim, channel);
}

/**
 * @brief Turn the buzzer on at the previously set frequency (passive) or
 *        simply enable the output (active).
 *
 * Active mode: drives the GPIO pin high, powering the buzzer's internal
 * oscillator. The pitch is fixed by the buzzer hardware.
 *
 * Passive mode: sets the PWM compare register to 50% of the current period
 * (square wave) at whatever frequency Buzzer_Tone() last programmed. If
 * Buzzer_Tone() has never been called, the frequency is whatever the timer
 * was initialized with (typically silent/0 Hz).
 *
 * @param buzzer  Buzzer instance (NULL is safely ignored).
 */
void Buzzer_On(Buzzer_t *buzzer)
{
    if (buzzer == NULL) {
        return;
    }
    if (buzzer->mode == BUZZER_MODE_ACTIVE_GPIO) {
        GPIO_Driver_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_SET);
    } else if (buzzer->mode == BUZZER_MODE_PASSIVE_PWM) {
        /* Drive a square wave at whatever frequency Buzzer_Tone last set. */
        uint32_t period = __HAL_TIM_GET_AUTORELOAD(buzzer->htim);
        __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, (period + 1U) / 2U);
    }
}

/**
 * @brief Turn the buzzer off (silence).
 *
 * Active mode: drives the GPIO pin low, cutting power to the buzzer.
 * Passive mode: sets the PWM compare register to 0, holding the output low.
 *
 * @param buzzer  Buzzer instance (NULL is safely ignored).
 */
void Buzzer_Off(Buzzer_t *buzzer)
{
    if (buzzer == NULL) {
        return;
    }
    if (buzzer->mode == BUZZER_MODE_ACTIVE_GPIO) {
        GPIO_Driver_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_RESET);
    } else if (buzzer->mode == BUZZER_MODE_PASSIVE_PWM) {
        __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, 0);
    }
}

/**
 * @brief Set the output frequency and duty cycle (passive mode only).
 *
 * This function computes the timer prescaler (PSC) and auto-reload register
 * (ARR) to generate the requested frequency with the requested duty cycle.
 *
 * Algorithm:
 *  1. ticks = timerClockHz / frequencyHz  (timer clock cycles per period)
 *  2. If ticks < 2, frequency is too high to resolve a duty cycle -> error.
 *  3. Because ARR is 16-bit (max 65535), we may need a prescaler:
 *     prescaler = (ticks - 1) / 65536
 *     If prescaler > 65535, frequency is too low for this timer clock -> error.
 *  4. period = (ticks / (prescaler + 1)) - 1  (this is the ARR value)
 *  5. pulse  = (period + 1) * dutyPercent / 100  (this is the CCR value)
 *  6. Write PSC, ARR, CCR, then generate an update event (UG) to load the
 *     shadow registers immediately so the new frequency takes effect without
 *     waiting for the next overflow.
 *
 * @param buzzer       Buzzer instance (must be in passive mode, not NULL).
 * @param frequencyHz  Desired frequency in Hz. Must be >= 1 and <= timerClockHz/2.
 * @param dutyPercent  Duty cycle 0..100. 50 gives a square wave (loudest/cleanest).
 * @return HAL_OK on success, HAL_ERROR if:
 *         - buzzer is NULL or not in passive mode
 *         - frequencyHz == 0 or dutyPercent > 100
 *         - frequency too high (ticks < 2) or too low (prescaler > 65535)
 */
HAL_StatusTypeDef Buzzer_Tone(Buzzer_t *buzzer, uint32_t frequencyHz, uint8_t dutyPercent)
{
    if (buzzer == NULL || buzzer->mode != BUZZER_MODE_PASSIVE_PWM) {
        return HAL_ERROR;
    }
    if (frequencyHz == 0 || dutyPercent > 100) {
        return HAL_ERROR;
    }

    uint32_t timerClk = buzzer->pwmTimerClockHz;

    /* Ticks of the timer input clock in one period, before prescaling. */
    uint32_t ticks = timerClk / frequencyHz;
    if (ticks < 2U) {
        return HAL_ERROR;   /* frequency too high to resolve a duty cycle */
    }

    /* ARR is 16 bit on every F4 timer, so audio frequencies need a prescaler:
       at 84 MHz a 1 kHz tone is 84000 ticks, which would wrap ARR silently. */
    uint32_t prescaler = (ticks - 1U) / 65536U;
    if (prescaler > 65535U) {
        return HAL_ERROR;   /* frequency too low for this timer clock */
    }

    uint32_t period = (ticks / (prescaler + 1U)) - 1U;
    uint32_t pulse = (period + 1U) * dutyPercent / 100U;

    __HAL_TIM_SET_PRESCALER(buzzer->htim, prescaler);
    __HAL_TIM_SET_AUTORELOAD(buzzer->htim, period);
    __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, pulse);

    /* Generate an update event to load shadow registers (PSC, ARR, CCR) immediately. */
    buzzer->htim->Instance->EGR = TIM_EGR_UG;

    return HAL_OK;
}

/**
 * @brief Check whether the buzzer is currently producing sound.
 *
 * Active mode: reads the GPIO output data register (ODR).
 * Passive mode: checks whether the PWM compare register (CCR) is non-zero.
 *
 * @param buzzer  Buzzer instance (NULL returns false).
 * @return true if on, false if off or buzzer is NULL.
 */
bool Buzzer_IsOn(const Buzzer_t *buzzer)
{
    if (buzzer == NULL) {
        return false;
    }
    if (buzzer->mode == BUZZER_MODE_ACTIVE_GPIO) {
        return GPIO_Driver_ReadPin(buzzer->port, buzzer->pin) == GPIO_PIN_SET;
    }
    if (buzzer->mode == BUZZER_MODE_PASSIVE_PWM) {
        return __HAL_TIM_GET_COMPARE(buzzer->htim, buzzer->channel) > 0;
    }

    return false;
}
