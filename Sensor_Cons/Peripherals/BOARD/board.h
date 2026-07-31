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

/* On-board user button -------------------------------------------------------*/
#define BOARD_BUTTON_PORT       GPIOA
#define BOARD_BUTTON_PIN        GPIO_PIN_0
#define BOARD_BUTTON_ACTIVE_LOW false

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
