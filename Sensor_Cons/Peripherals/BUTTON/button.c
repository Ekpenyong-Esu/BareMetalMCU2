/**
 ******************************************************************************
 * @file    button.c
 * @brief   Simplified Button implementation for STM32F429
 * @details Streamlined button driver with essential functionality only
 * @version 2.0
 * @date    2025-09-27
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "button.h"
#include "gpio.h"

/* Private functions ---------------------------------------------------------*/

/* Reads the pin without the initialized guard, so init can seed the state. */
static ButtonState_t Button_ReadPin(const ButtonConfig_t *config) {
    GPIO_PinState pinState = GPIO_Driver_ReadPin(config->port, config->pin);
    GPIO_PinState activeLevel = config->activeLow ? GPIO_PIN_RESET : GPIO_PIN_SET;

    return (pinState == activeLevel) ? BUTTON_PRESSED : BUTTON_RELEASED;
}

static bool Button_GPIO_Init(const ButtonConfig_t *config) {
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = config->pin;
    /* Both edges: the driver latches press and release, and a falling-only
       line would never signal a press on an active-high button. */
    gpioInit.Mode = config->enableInterrupt ? GPIO_MODE_IT_RISING_FALLING : GPIO_MODE_INPUT;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    gpioInit.Pull = config->activeLow ? GPIO_PULLUP : GPIO_PULLDOWN;

    if (GPIO_Driver_Pin_Init(config->port, &gpioInit) != HAL_OK) {
        return false;
    }

    if (config->enableInterrupt &&
        GPIO_Driver_EnableIRQ(config->pin, BUTTON_IRQ_PRIORITY, 0) != HAL_OK) {
        return false;
    }

    return true;
}

/* Exported functions --------------------------------------------------------*/

bool Button_Init(ButtonHandle_t *handle, GPIO_TypeDef *port, uint16_t pin) {
    ButtonConfig_t config = {.port = port,
                             .pin = pin,
                             .activeLow = true,
                             .debounceMs = BUTTON_DEBOUNCE_DEFAULT,
                             .enableInterrupt = true};

    return Button_InitCustom(handle, &config);
}

bool Button_InitCustom(ButtonHandle_t *handle, const ButtonConfig_t *config) {
    if (handle == NULL || config == NULL || config->port == NULL) {
        return false;
    }

    handle->initialized = false;
    handle->config = *config;
    handle->lastChangeTime = HAL_GetTick();
    handle->pressEvent = false;
    handle->releaseEvent = false;

    if (!Button_GPIO_Init(&handle->config)) {
        return false;
    }

    handle->state = Button_ReadPin(&handle->config);
    handle->lastState = handle->state;
    handle->initialized = true;

    return true;
}

ButtonState_t Button_Read(ButtonHandle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return BUTTON_RELEASED;
    }

    ButtonState_t rawState = Button_ReadPin(&handle->config);
    uint32_t currentTime = HAL_GetTick();

    if (rawState != handle->lastState) {
        handle->lastChangeTime = currentTime;
        handle->lastState = rawState;
    }

    if ((currentTime - handle->lastChangeTime) >= handle->config.debounceMs &&
        rawState != handle->state) {
        /* Latch the edge so each Was* query can consume it independently. */
        if (rawState == BUTTON_PRESSED) {
            handle->pressEvent = true;
        }
        else {
            handle->releaseEvent = true;
        }
        handle->state = rawState;
    }

    return handle->state;
}

bool Button_IsPressed(ButtonHandle_t *handle) {
    return (Button_Read(handle) == BUTTON_PRESSED);
}

bool Button_WasPressed(ButtonHandle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    Button_Read(handle);

    bool pressed = handle->pressEvent;
    handle->pressEvent = false;
    return pressed;
}

bool Button_WasReleased(ButtonHandle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    Button_Read(handle);

    bool released = handle->releaseEvent;
    handle->releaseEvent = false;
    return released;
}

ButtonState_t Button_ReadRaw(ButtonHandle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return BUTTON_RELEASED;
    }

    return Button_ReadPin(&handle->config);
}
