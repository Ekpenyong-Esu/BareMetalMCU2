/**
 * @file keypad.c
 * @brief Keypad lifetime and key map configuration
 */

#include "keypad_core.h"
#include "keypad_scan.h"
#include <string.h>

static const char KEYPAD_DEFAULT_KEYMAP[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};

static bool Keypad_ValidateConfig(const KeypadConfig_t *config) {
    for (uint8_t i = 0U; i < KEYPAD_ROWS; i++) {
        if (config->rows[i].port == NULL || config->rows[i].pin == 0U) {
            return false;
        }
    }

    for (uint8_t i = 0U; i < KEYPAD_COLS; i++) {
        if (config->cols[i].port == NULL || config->cols[i].pin == 0U) {
            return false;
        }
    }

    return true;
}

bool Keypad_Init(KeypadHandle_t *handle, const KeypadConfig_t *config) {
    if (handle == NULL || config == NULL) {
        return false;
    }

    /* Cleared first so a rejected configuration cannot leave a handle that
       still reports itself as initialized. */
    memset(handle, 0, sizeof(*handle));

    if (!Keypad_ValidateConfig(config)) {
        return false;
    }

    handle->config = *config;

    if (handle->config.debounceMs == 0U) {
        handle->config.debounceMs = KEYPAD_DEBOUNCE_MS;
    }

    memcpy(handle->keyMap, KEYPAD_DEFAULT_KEYMAP, sizeof(handle->keyMap));

    handle->lastKey = KEYPAD_NO_KEY;
    handle->currentKey = KEYPAD_NO_KEY;
    handle->lastKeyTime = HAL_GetTick();

    if (!Keypad_Scan_GpioInit(&handle->config)) {
        return false;
    }

    handle->initialized = true;

    return true;
}

bool Keypad_DeInit(KeypadHandle_t *handle) {
    if (handle == NULL) {
        return false;
    }

    if (!handle->initialized) {
        return true;
    }

    Keypad_Scan_GpioDeInit(&handle->config);
    memset(handle, 0, sizeof(*handle));

    return true;
}

bool Keypad_SetKeyMap(KeypadHandle_t *handle, const char keyMap[KEYPAD_ROWS][KEYPAD_COLS]) {
    if (handle == NULL || keyMap == NULL || !handle->initialized) {
        return false;
    }

    memcpy(handle->keyMap, keyMap, sizeof(handle->keyMap));

    return true;
}

bool Keypad_IsInitialized(const KeypadHandle_t *handle) {
    return (handle != NULL) && handle->initialized;
}
