/**
 * @file keypad_core.h
 * @brief Keypad lifetime and key map configuration
 */

#ifndef KEYPAD_CORE_H
#define KEYPAD_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "keypad_types.h"

/**
 * @brief   Initialize a keypad with the given pin configuration
 * @details Rows are driven open drain and columns are pulled up, so no two keys
 *          can short one row output against another.
 * @param   handle Handle to initialize
 * @param   config Pin configuration
 * @retval  bool True on success
 */
bool Keypad_Init(KeypadHandle_t *handle, const KeypadConfig_t *config);

/**
 * @brief   Release the keypad and return every line to a high impedance input
 * @param   handle Handle to release
 * @retval  bool True on success
 */
bool Keypad_DeInit(KeypadHandle_t *handle);

/**
 * @brief   Replace the key mapping
 * @param   handle Handle
 * @param   keyMap Row-major 4x4 array of key characters
 * @retval  bool True on success
 * @note    Default mapping: 1-9, 0, A-D, *, #
 */
bool Keypad_SetKeyMap(KeypadHandle_t *handle, const char keyMap[KEYPAD_ROWS][KEYPAD_COLS]);

/**
 * @brief   Check whether the handle has been initialized
 */
bool Keypad_IsInitialized(const KeypadHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_CORE_H */
