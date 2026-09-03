/**
  ******************************************************************************
  * @file    relay.h
  * @brief   Relay driver — turn a relay on and off with one GPIO pin
  * @details A relay is an electrically controlled switch. The MCU drives a
  *          GPIO pin high or low to open or close the relay contacts. Some
  *          relay boards are active-high (high = on), others are active-low
  *          (low = on), so the polarity is part of the config.
  *
  * How it works (in simple words):
  * - Relay_Init sets up the GPIO pin as an output and starts with relay off.
  * - Relay_On / Relay_Off drive the pin to the right level for the polarity.
  * - Relay_IsOn reads the pin back to check the current state.
  */

#ifndef RELAY_H
#define RELAY_H

#include "stm32f4xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief How the relay turns on
 */
typedef enum {
    RELAY_ACTIVE_HIGH = 0,  /*!< Pin high = relay on */
    RELAY_ACTIVE_LOW        /*!< Pin low = relay on */
} RelayPolarity_t;

/**
 * @brief Holds everything for one relay
 */
typedef struct {
    GPIO_TypeDef *port;     /*!< GPIO port the relay is wired to */
    uint16_t pin;           /*!< GPIO pin for the relay */
    RelayPolarity_t polarity; /*!< Whether high or low means "on" */
} Relay_t;

/**
 * @brief Set up a relay pin
 * @param relay     Handle to fill in (must not be NULL)
 * @param port      GPIO port
 * @param pin       GPIO pin
 * @param polarity  Active-high or active-low
 * @retval HAL_OK if ready, HAL_ERROR if something is wrong
 * @note  Starts with the relay off (safe state)
 */
HAL_StatusTypeDef Relay_Init(Relay_t *relay, GPIO_TypeDef *port, uint16_t pin, RelayPolarity_t polarity);

/**
 * @brief Turn the relay on
 * @param relay  Handle
 */
void Relay_On(Relay_t *relay);

/**
 * @brief Turn the relay off
 * @param relay  Handle
 */
void Relay_Off(Relay_t *relay);

/**
 * @brief Check if the relay is currently on
 * @param relay  Handle
 * @retval true if on, false if off or handle is NULL
 */
bool Relay_IsOn(const Relay_t *relay);

#ifdef __cplusplus
}
#endif

#endif /* RELAY_H */
