/**
 * @file keypad_input.h
 * @brief Key acquisition, debouncing and blocking helpers
 */

#ifndef KEYPAD_INPUT_H
#define KEYPAD_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "keypad_types.h"

/**
 * @brief   Scan the keypad and report a newly debounced key press
 * @details Non-blocking. Returns a character once per press, after the reading
 *          has been stable for the configured debounce interval.
 * @param   handle Handle
 * @retval  char Key character, or KEYPAD_NO_KEY
 */
char Keypad_GetKey(KeypadHandle_t *handle);

/**
 * @brief   Scan the keypad without debouncing
 * @param   handle Handle
 * @retval  char Key character, or KEYPAD_NO_KEY
 */
char Keypad_GetKeyRaw(KeypadHandle_t *handle);

/**
 * @brief   Check whether any key is currently held down
 * @param   handle Handle
 * @retval  bool True if a key is pressed
 */
bool Keypad_IsKeyPressed(KeypadHandle_t *handle);

/**
 * @brief   Block until a key is pressed and released, or the timeout elapses
 * @param   handle    Handle
 * @param   timeoutMs Timeout in milliseconds, HAL_MAX_DELAY to wait forever
 * @retval  char Key character, or KEYPAD_NO_KEY on timeout
 */
char Keypad_WaitForKey(KeypadHandle_t *handle, uint32_t timeoutMs);

/**
 * @brief   Report the position of the key currently held down
 * @param   handle Handle
 * @param   row    Receives the row index, 0..KEYPAD_ROWS-1
 * @param   col    Receives the column index, 0..KEYPAD_COLS-1
 * @retval  bool True if a key is pressed
 */
bool Keypad_GetKeyPosition(KeypadHandle_t *handle, uint8_t *row, uint8_t *col);

/**
 * @brief   Read the key most recently reported by Keypad_GetKey()
 * @param   handle Handle
 * @retval  char Key character, or KEYPAD_NO_KEY
 */
char Keypad_GetLastKey(const KeypadHandle_t *handle);

/**
 * @brief   Forget the last reported key
 * @param   handle Handle
 */
void Keypad_ClearLastKey(KeypadHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_INPUT_H */
