/**
 * @file keypad_types.h
 * @brief Data types and constants for the 4x4 matrix keypad driver
 */

#ifndef KEYPAD_TYPES_H
#define KEYPAD_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define KEYPAD_ROWS 4U         /**< Number of rows */
#define KEYPAD_COLS 4U         /**< Number of columns */
#define KEYPAD_DEBOUNCE_MS 20U /**< Default debounce time in milliseconds */
#define KEYPAD_NO_KEY '\0'     /**< No key pressed */

/**
 * @brief One keypad line
 */
typedef struct {
    GPIO_TypeDef *port; /**< GPIO port */
    uint16_t pin;       /**< GPIO pin mask */
} KeypadPin_t;

/**
 * @brief Keypad configuration
 */
typedef struct {
    KeypadPin_t rows[KEYPAD_ROWS]; /**< Row pins, driven open-drain during a scan */
    KeypadPin_t cols[KEYPAD_COLS]; /**< Column pins, inputs with pull-ups */
    uint32_t debounceMs;           /**< Debounce time in milliseconds, 0 selects the default */
} KeypadConfig_t;

/**
 * @brief Keypad handle
 */
typedef struct {
    KeypadConfig_t config;                 /**< Keypad configuration */
    char keyMap[KEYPAD_ROWS][KEYPAD_COLS]; /**< Key mapping matrix */
    char lastKey;                          /**< Last reported key */
    char currentKey;                       /**< Reading currently being debounced */
    uint32_t lastKeyTime;                  /**< Tick at which the reading last changed */
    bool initialized;                      /**< Initialization flag */
} KeypadHandle_t;

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_TYPES_H */
