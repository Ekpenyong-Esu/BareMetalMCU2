/**
 * @file keypad_scan.c
 * @brief Internal GPIO matrix driving and sampling for the keypad
 */

#include "keypad_scan.h"
#include "gpio.h"

/* The column pull-ups are weak, so a released row needs a few RC time constants
   before the line reads high again. */
#define KEYPAD_SETTLE_US        10U
#define KEYPAD_CYCLES_PER_LOOP  4U
#define KEYPAD_HZ_PER_MHZ       1000000U

static void Keypad_Scan_Settle(void)
{
    volatile uint32_t count = (KEYPAD_SETTLE_US * (SystemCoreClock / KEYPAD_HZ_PER_MHZ)) /
                              KEYPAD_CYCLES_PER_LOOP;

    while (count-- > 0U) {
        __NOP();
    }
}

bool Keypad_Scan_GpioInit(const KeypadConfig_t *config)
{
    GPIO_InitTypeDef gpioInit = {0};

    /* Open drain: an idle row is high impedance, so two keys pressed in the same
       column cannot short a driven-high row against a driven-low one. */
    for (uint8_t i = 0U; i < KEYPAD_ROWS; i++) {
        gpioInit.Pin = config->rows[i].pin;
        gpioInit.Mode = GPIO_MODE_OUTPUT_OD;
        gpioInit.Pull = GPIO_NOPULL;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        if (GPIO_Driver_Pin_Init(config->rows[i].port, &gpioInit) != HAL_OK) {
            return false;
        }
        HAL_GPIO_WritePin(config->rows[i].port, config->rows[i].pin, GPIO_PIN_SET);
    }

    for (uint8_t i = 0U; i < KEYPAD_COLS; i++) {
        gpioInit.Pin = config->cols[i].pin;
        gpioInit.Mode = GPIO_MODE_INPUT;
        gpioInit.Pull = GPIO_PULLUP;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        if (GPIO_Driver_Pin_Init(config->cols[i].port, &gpioInit) != HAL_OK) {
            return false;
        }
    }

    return true;
}

void Keypad_Scan_GpioDeInit(const KeypadConfig_t *config)
{
    GPIO_InitTypeDef gpioInit = {0};

    for (uint8_t i = 0U; i < KEYPAD_ROWS; i++) {
        HAL_GPIO_WritePin(config->rows[i].port, config->rows[i].pin, GPIO_PIN_SET);
        gpioInit.Pin = config->rows[i].pin;
        gpioInit.Mode = GPIO_MODE_INPUT;
        gpioInit.Pull = GPIO_NOPULL;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_Driver_Pin_Init(config->rows[i].port, &gpioInit);
    }

    for (uint8_t i = 0U; i < KEYPAD_COLS; i++) {
        gpioInit.Pin = config->cols[i].pin;
        gpioInit.Mode = GPIO_MODE_INPUT;
        gpioInit.Pull = GPIO_NOPULL;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_Driver_Pin_Init(config->cols[i].port, &gpioInit);
    }
}

static void Keypad_Scan_ReleaseRows(const KeypadConfig_t *config)
{
    for (uint8_t i = 0U; i < KEYPAD_ROWS; i++) {
        HAL_GPIO_WritePin(config->rows[i].port, config->rows[i].pin, GPIO_PIN_SET);
    }
}

bool Keypad_Scan_Matrix(const KeypadConfig_t *config, uint8_t *row, uint8_t *col)
{
    bool found = false;

    if (config == NULL || row == NULL || col == NULL) {
        return false;
    }

    for (uint8_t r = 0U; r < KEYPAD_ROWS && !found; r++) {
        Keypad_Scan_ReleaseRows(config);
        HAL_GPIO_WritePin(config->rows[r].port, config->rows[r].pin, GPIO_PIN_RESET);
        Keypad_Scan_Settle();

        for (uint8_t c = 0U; c < KEYPAD_COLS; c++) {
            if (HAL_GPIO_ReadPin(config->cols[c].port, config->cols[c].pin) == GPIO_PIN_RESET) {
                *row = r;
                *col = c;
                found = true;
                break;
            }
        }
    }

    Keypad_Scan_ReleaseRows(config);

    return found;
}
