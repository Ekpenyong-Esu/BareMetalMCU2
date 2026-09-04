/**
 ******************************************************************************
 * @file    relay.c
 * @brief   Relay driver — simple on/off control for a relay
 * @details Drives one GPIO pin to switch a relay. Handles both active-high
 *          and active-low boards. Starts with the relay off for safety.
 */

#include "relay.h"
#include "gpio.h"

static inline GPIO_PinState relay_active_state(RelayPolarity_t polarity) {
    return (polarity == RELAY_ACTIVE_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

static inline GPIO_PinState relay_inactive_state(RelayPolarity_t polarity) {
    return (polarity == RELAY_ACTIVE_HIGH) ? GPIO_PIN_RESET : GPIO_PIN_SET;
}

HAL_StatusTypeDef Relay_Init(Relay_t *relay, GPIO_TypeDef *port, uint16_t pin,
                             RelayPolarity_t polarity) {
    if (relay == NULL || port == NULL) {
        return HAL_ERROR;
    }

    relay->port = port;
    relay->pin = pin;
    relay->polarity = polarity;

    if (GPIO_Driver_ClockEnable(port) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Drive the inactive level before the pin becomes an output: ODR resets to
       0, so an active-low relay would otherwise pull in during Init. */
    GPIO_PinState idleLevel = relay_inactive_state(polarity);
    GPIO_Driver_WritePin(relay->port, relay->pin, idleLevel);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    if (GPIO_Driver_Pin_Init(port, &GPIO_InitStruct) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

void Relay_On(Relay_t *relay) {
    GPIO_PinState activeState = GPIO_PIN_RESET;

    if (relay == NULL) {
        return;
    }
    activeState = relay_active_state(relay->polarity);
    GPIO_Driver_WritePin(relay->port, relay->pin, activeState);
}

void Relay_Off(Relay_t *relay) {
    GPIO_PinState inactiveState = GPIO_PIN_RESET;

    if (relay == NULL) {
        return;
    }
    inactiveState = relay_inactive_state(relay->polarity);
    GPIO_Driver_WritePin(relay->port, relay->pin, inactiveState);
}

bool Relay_IsOn(const Relay_t *relay) {
    GPIO_PinState activeState = GPIO_PIN_RESET;
    GPIO_PinState state = GPIO_PIN_RESET;

    if (relay == NULL) {
        return false;
    }
    activeState = relay_active_state(relay->polarity);
    state = GPIO_Driver_ReadPin(relay->port, relay->pin);
    return (state == activeState);
}
