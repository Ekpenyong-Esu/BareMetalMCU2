/**
 * @file    lcd_io.c
 * @brief   Low level HD44780 transport: GPIO wiring, timing and bus writes
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd_io.h"
#include "gpio.h"

/* Private define ------------------------------------------------------------*/

/** Cycles consumed by one iteration of the delay loop below */
#define LCD_DELAY_LOOP_CYCLES 4U

/** Mask selecting the data bit that drives pin Dn of the parallel bus */
#define LCD_DATA_BIT(n) ((uint8_t)(1U << (n)))

/** Width of one half of a byte on the 4-bit bus */
#define LCD_NIBBLE_BITS 4U
#define LCD_NIBBLE_MASK 0x0FU

/* Private function prototypes -----------------------------------------------*/
static void LCD_IO_InitPin(GPIO_TypeDef *port, uint16_t pin);
static void LCD_IO_PulseEnable(LCD_HandleTypeDef *handle);
static void LCD_IO_WriteDataPin(const LCD_PinTypeDef *pin, uint8_t byte, uint8_t mask);

/* Public functions ----------------------------------------------------------*/

void LCD_IO_DelayUs(uint32_t micros) {
    const uint32_t cyclesPerUs = SystemCoreClock / 1000000U;
    volatile uint32_t count = (micros * cyclesPerUs) / LCD_DELAY_LOOP_CYCLES;

    while (count != 0U) {
        count--;
        __NOP();
    }
}

void LCD_IO_DelayMs(uint32_t millis) {
    HAL_Delay(millis);
}

void LCD_IO_SetPin(const LCD_PinTypeDef *pin, GPIO_PinState state) {
    if (pin->port != NULL) {
        HAL_GPIO_WritePin(pin->port, pin->pin, state);
    }
}

void LCD_IO_InitGPIO(LCD_HandleTypeDef *handle) {
    const LCD_PinsTypeDef *pins = &handle->config.pins;

    /* Control pins */
    LCD_IO_InitPin(pins->rs.port, pins->rs.pin);
    LCD_IO_SetPin(&pins->rs, GPIO_PIN_RESET);

    if (handle->config.useRW) {
        LCD_IO_InitPin(pins->rw.port, pins->rw.pin);
        LCD_IO_SetPin(&pins->rw, GPIO_PIN_RESET);
    }

    LCD_IO_InitPin(pins->en.port, pins->en.pin);
    LCD_IO_SetPin(&pins->en, GPIO_PIN_RESET);

    /* Data pins D4-D7 are used by both bus widths */
    LCD_IO_InitPin(pins->d4.port, pins->d4.pin);
    LCD_IO_InitPin(pins->d5.port, pins->d5.pin);
    LCD_IO_InitPin(pins->d6.port, pins->d6.pin);
    LCD_IO_InitPin(pins->d7.port, pins->d7.pin);

    if (handle->config.mode == LCD_MODE_8BIT) {
        LCD_IO_InitPin(pins->d0.port, pins->d0.pin);
        LCD_IO_InitPin(pins->d1.port, pins->d1.pin);
        LCD_IO_InitPin(pins->d2.port, pins->d2.pin);
        LCD_IO_InitPin(pins->d3.port, pins->d3.pin);
    }

    if (handle->config.useBacklight) {
        LCD_IO_InitPin(pins->backlight.port, pins->backlight.pin);
        LCD_IO_SetPin(&pins->backlight, GPIO_PIN_SET);
    }
}

void LCD_IO_WriteNibble(LCD_HandleTypeDef *handle, uint8_t nibble) {
    const LCD_PinsTypeDef *pins = &handle->config.pins;

    LCD_IO_WriteDataPin(&pins->d4, nibble, LCD_DATA_BIT(0));
    LCD_IO_WriteDataPin(&pins->d5, nibble, LCD_DATA_BIT(1));
    LCD_IO_WriteDataPin(&pins->d6, nibble, LCD_DATA_BIT(2));
    LCD_IO_WriteDataPin(&pins->d7, nibble, LCD_DATA_BIT(3));

    LCD_IO_PulseEnable(handle);
}

void LCD_IO_WriteByte(LCD_HandleTypeDef *handle, uint8_t byte, uint8_t regSelect) {
    const LCD_PinsTypeDef *pins = &handle->config.pins;

    LCD_IO_SetPin(&pins->rs, (regSelect != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    if (handle->config.useRW) {
        LCD_IO_SetPin(&pins->rw, GPIO_PIN_RESET);
    }

    if (handle->config.mode == LCD_MODE_8BIT) {
        LCD_IO_WriteDataPin(&pins->d0, byte, LCD_DATA_BIT(0));
        LCD_IO_WriteDataPin(&pins->d1, byte, LCD_DATA_BIT(1));
        LCD_IO_WriteDataPin(&pins->d2, byte, LCD_DATA_BIT(2));
        LCD_IO_WriteDataPin(&pins->d3, byte, LCD_DATA_BIT(3));
        LCD_IO_WriteDataPin(&pins->d4, byte, LCD_DATA_BIT(4));
        LCD_IO_WriteDataPin(&pins->d5, byte, LCD_DATA_BIT(5));
        LCD_IO_WriteDataPin(&pins->d6, byte, LCD_DATA_BIT(6));
        LCD_IO_WriteDataPin(&pins->d7, byte, LCD_DATA_BIT(7));

        LCD_IO_PulseEnable(handle);
    }
    else {
        LCD_IO_WriteNibble(handle, (uint8_t)((byte >> LCD_NIBBLE_BITS) & LCD_NIBBLE_MASK));
        LCD_IO_WriteNibble(handle, (uint8_t)(byte & LCD_NIBBLE_MASK));
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Configure a single LCD pin as push-pull output
 * @param   port GPIO port, NULL for an unwired pin
 * @param   pin  GPIO pin
 */
static void LCD_IO_InitPin(GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (port == NULL) {
        return;
    }

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Driver_Pin_Init(port, &GPIO_InitStruct);
}

/**
 * @brief   Latch the currently driven data pins into the panel
 * @param   handle LCD handle
 */
static void LCD_IO_PulseEnable(LCD_HandleTypeDef *handle) {
    const LCD_PinTypeDef *enable = &handle->config.pins.en;

    LCD_IO_SetPin(enable, GPIO_PIN_RESET);
    LCD_IO_DelayUs(LCD_ENABLE_PULSE_US);
    LCD_IO_SetPin(enable, GPIO_PIN_SET);
    LCD_IO_DelayUs(LCD_ENABLE_PULSE_US);
    LCD_IO_SetPin(enable, GPIO_PIN_RESET);
    LCD_IO_DelayUs(LCD_COMMAND_DELAY_US);
}

/**
 * @brief   Drive one data pin from the corresponding bit of a byte
 * @param   pin  Pin configuration
 * @param   byte Source byte
 * @param   mask Bit selecting this pin
 */
static void LCD_IO_WriteDataPin(const LCD_PinTypeDef *pin, uint8_t byte, uint8_t mask) {
    LCD_IO_SetPin(pin, ((byte & mask) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
