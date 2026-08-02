/**
  ******************************************************************************
  * @file    keypad.c
  * @brief   4x4 Matrix Keypad implementation for STM32F429
  * @details Configurable row/column GPIO pins with debouncing support
  * @version 1.0
  * @date    2026-01-03
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "keypad.h"
#include "gpio.h"

/* Private constants ---------------------------------------------------------*/

/** @brief Busy-wait iterations allowing a driven row to settle before sampling */
#define KEYPAD_SETTLE_LOOPS 10U

/** @brief Default keymap for 4x4 matrix keypad */
static const char DEFAULT_KEYMAP[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize GPIO pins for keypad
 * @param   config Pointer to keypad configuration
 */
static void Keypad_GPIO_Init(const KeypadConfig_t* config)
{
    GPIO_InitTypeDef gpioInit = {0};

    /* Initialize row pins as outputs (directly drive low for scanning) */
    /* GPIO driver enables the port clock for each row/column port */
    for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
        gpioInit.Pin = config->rows[i].pin;
        gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
        gpioInit.Pull = GPIO_NOPULL;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_Driver_Pin_Init(config->rows[i].port, &gpioInit);

        /* Set all rows high initially */
        HAL_GPIO_WritePin(config->rows[i].port, config->rows[i].pin, GPIO_PIN_SET);
    }

    /* Initialize column pins as inputs with pull-up resistors */
    for (uint8_t i = 0; i < KEYPAD_COLS; i++) {
        gpioInit.Pin = config->cols[i].pin;
        gpioInit.Mode = GPIO_MODE_INPUT;
        gpioInit.Pull = GPIO_PULLUP;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_Driver_Pin_Init(config->cols[i].port, &gpioInit);
    }
}

/**
 * @brief   Drive every row high (the idle state between scans)
 * @param   config Pointer to keypad configuration
 */
static void Keypad_SetAllRowsHigh(const KeypadConfig_t* config)
{
    for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
        HAL_GPIO_WritePin(config->rows[i].port, config->rows[i].pin, GPIO_PIN_SET);
    }
}

/**
 * @brief   Set a specific row low for scanning
 * @param   config Pointer to keypad configuration
 * @param   row Row index to set low
 */
static void Keypad_SetRowLow(const KeypadConfig_t* config, uint8_t row)
{
    Keypad_SetAllRowsHigh(config);
    HAL_GPIO_WritePin(config->rows[row].port, config->rows[row].pin, GPIO_PIN_RESET);
}

/**
 * @brief   Read column pin state
 * @param   config Pointer to keypad configuration
 * @param   col Column index to read
 * @retval  true if column is low (key pressed), false otherwise
 */
static bool Keypad_ReadColumn(const KeypadConfig_t* config, uint8_t col)
{
    return HAL_GPIO_ReadPin(config->cols[col].port, config->cols[col].pin) == GPIO_PIN_RESET;
}

/**
 * @brief   Scan the keypad matrix
 * @param   config Pointer to keypad configuration
 * @param   row Pointer to store detected row
 * @param   col Pointer to store detected column
 * @retval  true if a key is detected, false otherwise
 */
static bool Keypad_ScanMatrix(const KeypadConfig_t* config, uint8_t* row, uint8_t* col)
{
    bool found = false;

    for (uint8_t r = 0; r < KEYPAD_ROWS && !found; r++) {
        Keypad_SetRowLow(config, r);

        /* Let the pull-ups settle before sampling the columns */
        for (volatile uint32_t i = 0; i < KEYPAD_SETTLE_LOOPS; i++) {
            __NOP();
        }

        for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
            if (Keypad_ReadColumn(config, c)) {
                *row = r;
                *col = c;
                found = true;
                break;
            }
        }
    }

    Keypad_SetAllRowsHigh(config);

    return found;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize keypad with configuration
 * @param   handle Pointer to keypad handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Keypad_Init(KeypadHandle_t* handle, const KeypadConfig_t* config)
{
    if (handle == NULL || config == NULL) {
        return false;
    }

    /* Validate configuration */
    for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
        if (config->rows[i].port == NULL) {
            return false;
        }
    }
    for (uint8_t i = 0; i < KEYPAD_COLS; i++) {
        if (config->cols[i].port == NULL) {
            return false;
        }
    }

    /* Copy configuration */
    handle->config = *config;

    /* Set default debounce if not specified */
    if (handle->config.debounceMs == 0) {
        handle->config.debounceMs = KEYPAD_DEBOUNCE_MS;
    }

    /* Set default keymap */
    for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
        for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
            handle->keyMap[r][c] = DEFAULT_KEYMAP[r][c];
        }
    }

    /* Initialize state */
    handle->lastKey = KEYPAD_NO_KEY;
    handle->currentKey = KEYPAD_NO_KEY;
    handle->lastKeyTime = 0;
    handle->initialized = false;

    /* Initialize GPIO */
    Keypad_GPIO_Init(&handle->config);

    handle->initialized = true;
    return true;
}

/**
 * @brief   Set custom key mapping
 * @param   handle Pointer to keypad handle
 * @param   keyMap 4x4 array of key characters
 * @retval  true if successful, false otherwise
 */
bool Keypad_SetKeyMap(KeypadHandle_t* handle, const char keyMap[KEYPAD_ROWS][KEYPAD_COLS])
{
    if (handle == NULL || keyMap == NULL) {
        return false;
    }

    for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
        for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
            handle->keyMap[r][c] = keyMap[r][c];
        }
    }

    return true;
}

/**
 * @brief   Scan keypad and get pressed key (blocking with debounce)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key, or KEYPAD_NO_KEY if none
 */
char Keypad_GetKey(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    uint8_t row, col;
    char key = KEYPAD_NO_KEY;

    /* Scan for pressed key */
    if (Keypad_ScanMatrix(&handle->config, &row, &col)) {
        key = handle->keyMap[row][col];
    }

    uint32_t currentTime = HAL_GetTick();

    /* Reading changed: restart the debounce window */
    if (key != handle->currentKey) {
        handle->currentKey = key;
        handle->lastKeyTime = currentTime;
        return KEYPAD_NO_KEY;
    }

    /* Only act once the reading has been stable for the debounce interval */
    if ((currentTime - handle->lastKeyTime) >= handle->config.debounceMs) {
        if (key == KEYPAD_NO_KEY) {
            /* A stable release re-arms the next press */
            handle->lastKey = KEYPAD_NO_KEY;
        } else if (key != handle->lastKey) {
            handle->lastKey = key;
            return key;
        }
    }

    return KEYPAD_NO_KEY;
}

/**
 * @brief   Scan keypad without debouncing (raw read)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key, or KEYPAD_NO_KEY if none
 */
char Keypad_GetKeyRaw(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    uint8_t row, col;
    if (Keypad_ScanMatrix(&handle->config, &row, &col)) {
        return handle->keyMap[row][col];
    }

    return KEYPAD_NO_KEY;
}

/**
 * @brief   Check if any key is currently pressed
 * @param   handle Pointer to keypad handle
 * @retval  true if a key is pressed, false otherwise
 */
bool Keypad_IsKeyPressed(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    uint8_t row, col;
    return Keypad_ScanMatrix(&handle->config, &row, &col);
}

/**
 * @brief   Wait for a key press (blocking)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key
 */
char Keypad_WaitForKey(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    char key = KEYPAD_NO_KEY;

    /* Wait until a key is pressed and debounced */
    while (key == KEYPAD_NO_KEY) {
        key = Keypad_GetKey(handle);
    }

    /* Wait for key release */
    while (Keypad_IsKeyPressed(handle)) {
        HAL_Delay(1);
    }

    return key;
}

/**
 * @brief   Get the row and column of pressed key
 * @param   handle Pointer to keypad handle
 * @param   row Pointer to store row index (0-3)
 * @param   col Pointer to store column index (0-3)
 * @retval  true if a key is pressed, false otherwise
 */
bool Keypad_GetKeyPosition(KeypadHandle_t* handle, uint8_t* row, uint8_t* col)
{
    if (handle == NULL || !handle->initialized || row == NULL || col == NULL) {
        return false;
    }

    return Keypad_ScanMatrix(&handle->config, row, col);
}

/**
 * @brief   Get last pressed key (non-blocking)
 * @param   handle Pointer to keypad handle
 * @retval  Last pressed key character, or KEYPAD_NO_KEY if none
 */
char Keypad_GetLastKey(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    return handle->lastKey;
}

/**
 * @brief   Clear last key buffer
 * @param   handle Pointer to keypad handle
 */
void Keypad_ClearLastKey(KeypadHandle_t* handle)
{
    if (handle != NULL && handle->initialized) {
        handle->lastKey = KEYPAD_NO_KEY;
    }
}
