/**
 * @file    counter_display.c
 * @brief   LED display for the button event counter.
 *
 * Single responsibility: drive the LEDs. The green LED mirrors the button
 * state; the red and aux LEDs show the press count in binary.
 */

#include "counter_display.h"

#include "board.h"

bool CounterDisplay_Init(CounterDisplay_t* handle)
{
    if (handle == NULL) {
        return false;
    }

    const LedConfig_t stateLed = {
        .port = BOARD_LED_GREEN_PORT,
        .pin = BOARD_LED_GREEN_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };

    const LedConfig_t countLed0 = {
        .port = BOARD_LED_RED_PORT,
        .pin = BOARD_LED_RED_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };
    
    const LedConfig_t countLed1 = {
        .port = BOARD_LED_AUX_PORT,
        .pin = BOARD_LED_AUX_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };

    return (Led_InitCustom(&handle->stateLed, &stateLed) &&
            Led_InitCustom(&handle->countLed0, &countLed0) &&
            Led_InitCustom(&handle->countLed1, &countLed1)) != 0;
}

void CounterDisplay_Show(CounterDisplay_t* handle, bool pressed, uint32_t count)
{
    if (handle == NULL) {
        return;
    }

    /* State LED mirrors the button: on while pressed, off while released. */
    if (pressed) {
        Led_SetState(&handle->stateLed, LED_ON);
    } else {
        Led_SetState(&handle->stateLed, LED_OFF);
    }

    /* Count LEDs show the count in binary (bit 0 = red, bit 1 = aux). */
    if ((count & 0x1u) != 0u) {
        Led_SetState(&handle->countLed0, LED_ON);
    } else {
        Led_SetState(&handle->countLed0, LED_OFF);
    }

    if ((count & 0x2u) != 0u) {
        Led_SetState(&handle->countLed1, LED_ON);
    } else {
        Led_SetState(&handle->countLed1, LED_OFF);
    }
}
