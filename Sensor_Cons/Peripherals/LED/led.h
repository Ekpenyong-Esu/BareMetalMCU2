/**
 * @file led.h
 * @brief LED state driver for STM32F429
 * @details Owns one responsibility: driving a GPIO-connected LED on or off.
 *          Timing-based behaviour lives in separate modules:
 *            - led_blink.h : fixed-rate blinking
 *            - led_software_pwm.h : software PWM (brightness / fading)
 *          Board pin assignments live in board.h.
 * @version 3.0
 * @date 2026-07-31
 */

#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief LED state enumeration
 */
typedef enum {
    LED_OFF = 0, /**< LED is off */
    LED_ON = 1   /**< LED is on */
} LedState_t;

/**
 * @brief LED configuration structure
 */
typedef struct {
    GPIO_TypeDef *port; /**< GPIO port */
    uint16_t pin;       /**< GPIO pin */
    bool activeLow;     /**< True if LED is active low */
} LedConfig_t;

/**
 * @brief LED handle structure
 */
typedef struct {
    LedConfig_t config; /**< LED configuration */
    LedState_t state;   /**< Current LED state */
    bool initialized;   /**< Initialization flag */
} LedHandle_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize LED with default settings (active high)
 * @param   handle Pointer to LED handle
 * @param   port GPIO port
 * @param   pin GPIO pin number
 * @retval  true if successful, false otherwise
 */
bool Led_Init(LedHandle_t *handle, GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief   Initialize LED with custom configuration
 * @param   handle Pointer to LED handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Led_InitCustom(LedHandle_t *handle, const LedConfig_t *config);

/**
 * @brief   Turn LED on
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_On(LedHandle_t *handle);

/**
 * @brief   Turn LED off
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Off(LedHandle_t *handle);

/**
 * @brief   Toggle LED state
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Toggle(LedHandle_t *handle);

/**
 * @brief   Set LED to specific state
 * @param   handle Pointer to LED handle
 * @param   state LED state to set
 * @retval  true if successful, false otherwise
 */
bool Led_SetState(LedHandle_t *handle, LedState_t state);

/**
 * @brief   Get current LED state
 * @param   handle Pointer to LED handle
 * @retval  Current LED state
 */
LedState_t Led_GetState(const LedHandle_t *handle);

/**
 * @brief   Check if LED is currently on
 * @param   handle Pointer to LED handle
 * @retval  true if LED is on, false otherwise
 */
bool Led_IsOn(const LedHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
