/**
  ******************************************************************************
  * @file    button_simple.c
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

static void Button_GPIO_Init(const ButtonConfig_t* config)
{
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = config->pin;
    gpioInit.Mode = config->enableInterrupt ? GPIO_MODE_IT_FALLING : GPIO_MODE_INPUT;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    gpioInit.Pull = config->activeLow ? GPIO_PULLUP : GPIO_PULLDOWN;

    GPIO_Driver_Pin_Init(config->port, &gpioInit);

    if (config->enableInterrupt) {
        uint32_t irq = 0;
        if (config->pin == GPIO_PIN_0) irq = EXTI0_IRQn;
        else if (config->pin == GPIO_PIN_1) irq = EXTI1_IRQn;
        else if (config->pin == GPIO_PIN_2) irq = EXTI2_IRQn;
        else if (config->pin == GPIO_PIN_3) irq = EXTI3_IRQn;
        else if (config->pin == GPIO_PIN_4) irq = EXTI4_IRQn;
        else if (config->pin >= GPIO_PIN_5 && config->pin <= GPIO_PIN_9) irq = EXTI9_5_IRQn;
        else if (config->pin >= GPIO_PIN_10 && config->pin <= GPIO_PIN_15) irq = EXTI15_10_IRQn;
        if (irq) {
            HAL_NVIC_SetPriority(irq, 2, 0);
            HAL_NVIC_EnableIRQ(irq);
        }
    }
}

/* Exported functions --------------------------------------------------------*/

bool Button_Init(ButtonHandle_t* handle, GPIO_TypeDef* port, uint16_t pin)
{
    ButtonConfig_t config = {
        .port = port,
        .pin = pin,
        .activeLow = true,
        .debounceMs = BUTTON_DEBOUNCE_DEFAULT,
        .enableInterrupt = true
    };

    return Button_InitCustom(handle, &config);
}

bool Button_InitCustom(ButtonHandle_t* handle, const ButtonConfig_t* config)
{
    if (handle == NULL || config == NULL || config->port == NULL) {
        return false;
    }

    handle->config = *config;
    handle->state = BUTTON_RELEASED;
    handle->lastState = BUTTON_RELEASED;
    handle->lastChangeTime = 0;
    handle->initialized = false;

    Button_GPIO_Init(&handle->config);

    handle->state = Button_ReadRaw(handle);
    handle->lastState = handle->state;
    handle->lastChangeTime = HAL_GetTick();
    handle->initialized = true;

    return true;
}

ButtonState_t Button_Read(ButtonHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return BUTTON_RELEASED;
    }

    ButtonState_t rawState = Button_ReadRaw(handle);
    uint32_t currentTime = HAL_GetTick();

    if (rawState != handle->lastState) {
        handle->lastChangeTime = currentTime;
        handle->lastState = rawState;
    }

    if ((currentTime - handle->lastChangeTime) >= handle->config.debounceMs) {
        handle->state = rawState;
    }

    return handle->state;
}

bool Button_IsPressed(ButtonHandle_t* handle)
{
    return (Button_Read(handle) == BUTTON_PRESSED);
}

bool Button_WasPressed(ButtonHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    ButtonState_t oldState = handle->state;
    ButtonState_t newState = Button_Read(handle);

    return (oldState == BUTTON_RELEASED && newState == BUTTON_PRESSED);
}

bool Button_WasReleased(ButtonHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    ButtonState_t oldState = handle->state;
    ButtonState_t newState = Button_Read(handle);

    return (oldState == BUTTON_PRESSED && newState == BUTTON_RELEASED);
}

ButtonState_t Button_ReadRaw(ButtonHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return BUTTON_RELEASED;
    }

    GPIO_PinState pinState = HAL_GPIO_ReadPin(handle->config.port, handle->config.pin);

    /* Determine button state based on active level */
    if (handle->config.activeLow) {
        return (pinState == GPIO_PIN_RESET) ? BUTTON_PRESSED : BUTTON_RELEASED;
    }

    return (pinState == GPIO_PIN_SET) ? BUTTON_PRESSED : BUTTON_RELEASED;
}
