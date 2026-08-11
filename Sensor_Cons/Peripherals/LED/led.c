/**
 * @file led.c
 * @brief LED state driver implementation for STM32F429
 * @version 3.0
 * @date 2026-07-31
 */

/* Includes ------------------------------------------------------------------*/
#include "led.h"
#include "gpio.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Configure the LED pin as a push-pull output
 * @param   config Pointer to LED configuration
 */
static void Led_GPIO_Init(const LedConfig_t* config)
{
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = config->pin;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    gpioInit.Pull = GPIO_NOPULL;

    GPIO_Driver_Pin_Init(config->port, &gpioInit);
}

/**
 * @brief   Set physical LED state
 * @param   handle Pointer to LED handle
 * @param   state LED state to set
 */
static void Led_SetPhysical(LedHandle_t* handle, LedState_t state)
{
    GPIO_PinState pinState = GPIO_PIN_RESET;

    /* Determine pin state based on active level and desired LED state */
    if (handle->config.activeLow) {
        pinState = (state == LED_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET;  // The LED is connected to power and the mcu drives it low to turn on
    } else {
        pinState = (state == LED_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    }

    GPIO_Driver_WritePin(handle->config.port, handle->config.pin, pinState);

    handle->state = state;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize LED with default settings (active high)
 * @param   handle Pointer to LED handle
 * @param   port GPIO port
 * @param   pin GPIO pin number
 * @retval  true if successful, false otherwise
 */
bool Led_Init(LedHandle_t* handle, GPIO_TypeDef* port, uint16_t pin)
{
    LedConfig_t config = {
        .port = port,
        .pin = pin,
        .activeLow = false
    };

    return Led_InitCustom(handle, &config);
}

/**
 * @brief   Initialize LED with custom configuration
 * @param   handle Pointer to LED handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Led_InitCustom(LedHandle_t* handle, const LedConfig_t* config)
{
    if (handle == NULL || config == NULL || config->port == NULL) {
        return false;
    }

    /* Copy configuration */
    handle->config = *config;

    /* Initialize state */
    handle->state = LED_OFF;
    handle->initialized = false;

    /* Initialize GPIO */
    Led_GPIO_Init(&handle->config);

    /* Set initial state (off) */
    Led_SetPhysical(handle, LED_OFF);

    handle->initialized = true;
    return true;
}

/**
 * @brief   Set LED to specific state
 * @param   handle Pointer to LED handle
 * @param   state LED state to set
 * @retval  true if successful, false otherwise
 */
bool Led_SetState(LedHandle_t* handle, LedState_t state)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    Led_SetPhysical(handle, state);
    return true;
}

/**
 * @brief   Turn LED on
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_On(LedHandle_t* handle)
{
    return Led_SetState(handle, LED_ON);
}

/**
 * @brief   Turn LED off
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Off(LedHandle_t* handle)
{
    return Led_SetState(handle, LED_OFF);
}

/**
 * @brief   Toggle LED state
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Toggle(LedHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    Led_SetPhysical(handle, (handle->state == LED_ON) ? LED_OFF : LED_ON);
    return true;
}

/**
 * @brief   Get current LED state
 * @param   handle Pointer to LED handle
 * @retval  Current LED state
 */
LedState_t Led_GetState(const LedHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return LED_OFF;
    }

    return handle->state;
}

/**
 * @brief   Check if LED is currently on
 * @param   handle Pointer to LED handle
 * @retval  true if LED is on, false otherwise
 */
bool Led_IsOn(const LedHandle_t* handle)
{
    return (Led_GetState(handle) == LED_ON);
}
