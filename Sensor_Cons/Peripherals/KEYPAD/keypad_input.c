/**
 * @file keypad_input.c
 * @brief Key acquisition, debouncing and blocking helpers
 */

#include "keypad_input.h"
#include "keypad_core.h"
#include "keypad_scan.h"

char Keypad_GetKeyRaw(KeypadHandle_t *handle) {
    uint8_t row = 0U;
    uint8_t col = 0U;

    if (!Keypad_IsInitialized(handle)) {
        return KEYPAD_NO_KEY;
    }

    if (!Keypad_Scan_Matrix(&handle->config, &row, &col)) {
        return KEYPAD_NO_KEY;
    }

    return handle->keyMap[row][col];
}

char Keypad_GetKey(KeypadHandle_t *handle) {
    char key = KEYPAD_NO_KEY;
    uint32_t now = 0U;

    if (!Keypad_IsInitialized(handle)) {
        return KEYPAD_NO_KEY;
    }

    key = Keypad_GetKeyRaw(handle);
    now = HAL_GetTick();

    /* Reading changed: restart the debounce window */
    if (key != handle->currentKey) {
        handle->currentKey = key;
        handle->lastKeyTime = now;
        return KEYPAD_NO_KEY;
    }

    /* Only act once the reading has been stable for the debounce interval */
    if ((now - handle->lastKeyTime) >= handle->config.debounceMs) {
        if (key == KEYPAD_NO_KEY) {
            /* A stable release re-arms the next press */
            handle->lastKey = KEYPAD_NO_KEY;
        }
        else if (key != handle->lastKey) {
            handle->lastKey = key;
            return key;
        }
    }

    return KEYPAD_NO_KEY;
}

bool Keypad_IsKeyPressed(KeypadHandle_t *handle) {
    uint8_t row = 0U;
    uint8_t col = 0U;

    if (!Keypad_IsInitialized(handle)) {
        return false;
    }

    return Keypad_Scan_Matrix(&handle->config, &row, &col);
}

bool Keypad_GetKeyPosition(KeypadHandle_t *handle, uint8_t *row, uint8_t *col) {
    if (!Keypad_IsInitialized(handle) || row == NULL || col == NULL) {
        return false;
    }

    return Keypad_Scan_Matrix(&handle->config, row, col);
}

char Keypad_WaitForKey(KeypadHandle_t *handle, uint32_t timeoutMs) {
    uint32_t start = 0;
    char key = KEYPAD_NO_KEY;

    if (!Keypad_IsInitialized(handle)) {
        return KEYPAD_NO_KEY;
    }

    start = HAL_GetTick();

    while (key == KEYPAD_NO_KEY) {
        key = Keypad_GetKey(handle);

        /* Without a bound a stuck key or a broken column would hang the caller. */
        if (key == KEYPAD_NO_KEY && timeoutMs != HAL_MAX_DELAY &&
            (HAL_GetTick() - start) >= timeoutMs) {
            return KEYPAD_NO_KEY;
        }
    }

    while (Keypad_IsKeyPressed(handle)) {
        if (timeoutMs != HAL_MAX_DELAY && (HAL_GetTick() - start) >= timeoutMs) {
            break;
        }
        HAL_Delay(1);
    }

    return key;
}

char Keypad_GetLastKey(const KeypadHandle_t *handle) {
    if (!Keypad_IsInitialized(handle)) {
        return KEYPAD_NO_KEY;
    }

    return handle->lastKey;
}

void Keypad_ClearLastKey(KeypadHandle_t *handle) {
    if (Keypad_IsInitialized(handle)) {
        handle->lastKey = KEYPAD_NO_KEY;
        handle->currentKey = KEYPAD_NO_KEY;
        handle->lastKeyTime = HAL_GetTick();
    }
}
