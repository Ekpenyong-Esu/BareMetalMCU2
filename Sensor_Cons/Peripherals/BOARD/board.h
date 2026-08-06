/**
 * @file    board.h
 * @brief   STM32F429I-DISC1 board pin map.
 * @details Board-specific wiring lives here so the peripheral drivers stay
 *          generic. Port this file when moving to another board.
 */

#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>

/* On-board LEDs -------------------------------------------------------------*/
#define BOARD_LED_GREEN_PORT    GPIOG
#define BOARD_LED_GREEN_PIN     GPIO_PIN_13
#define BOARD_LED_RED_PORT      GPIOG
#define BOARD_LED_RED_PIN       GPIO_PIN_14
#define BOARD_LED_ACTIVE_LOW    false

/* External LED on the extension header --------------------------------------*/
/* Port G carries no timer channel, so hardware PWM needs an off-board LED.
 * Wire: PA5 -> 330 ohm -> LED anode, cathode -> GND. */
#define BOARD_LED_EXT_PORT      GPIOA
#define BOARD_LED_EXT_PIN       GPIO_PIN_5
#define BOARD_LED_EXT_AF        GPIO_AF1_TIM2
#define BOARD_LED_EXT_TIM       TIM2
#define BOARD_LED_EXT_TIM_CH    TIM_CHANNEL_1

/* Second external LED, driven as a plain output -----------------------------*/
/* Wire: PB4 -> 330 ohm -> LED anode, cathode -> GND. */
#define BOARD_LED_AUX_PORT      GPIOB
#define BOARD_LED_AUX_PIN       GPIO_PIN_4

/* On-board user button -------------------------------------------------------*/
#define BOARD_BUTTON_PORT       GPIOA
#define BOARD_BUTTON_PIN        GPIO_PIN_0
#define BOARD_BUTTON_ACTIVE_LOW false

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
