/**
 * @file waveform_display.c
 * @brief Formats DAC waveform samples and prints them over USART1
 */

#include "waveform_display.h"

#include "dac.h"
#include "uart_blocking.h"
#include "uart_config.h"

#include <stdio.h>

#define WAVEFORM_DISPLAY_TX_SIZE  64

static const char kGreeting[] = "DAC waveform on PA4. Printing decimated samples.\r\n";

static char s_txLine[WAVEFORM_DISPLAY_TX_SIZE];

bool Waveform_DisplayInit(Waveform_Display_t *display)
{
    const UART_Config_t uartConfig = {
        .instance   = USART1,
        .baudRate   = UART_DEFAULT_BAUDRATE,
        .wordLength = UART_DEFAULT_WORDLENGTH,
        .stopBits   = UART_DEFAULT_STOPBITS,
        .parity     = UART_DEFAULT_PARITY,
        .mode       = UART_MODE_BLOCKING,
    };

    display->uart.huart = &display->halUart;

    if (UART_Blocking_Init(&display->uart, &uartConfig) != UART_OK) {
        return false;
    }

    UART_Blocking_Transmit(&display->uart, (const uint8_t *)kGreeting,
                           (uint16_t)(sizeof(kGreeting) - 1), HAL_MAX_DELAY);
    return true;
}

void Waveform_DisplayShow(Waveform_Display_t *display, uint32_t index, uint32_t code)
{
    /* %d.%03d keeps float formatting out of the printf library. */
    int millivolts = (int)(DAC_RawToVoltage(code) * 1000.0f);
    int written = snprintf(s_txLine, sizeof(s_txLine), "s[%2lu] = %4lu  (%d.%03d V)\r\n",
                           (unsigned long)index, (unsigned long)code,
                           millivolts / 1000, millivolts % 1000);

    if (written > 0) {
        UART_Blocking_Transmit(&display->uart, (const uint8_t *)s_txLine,
                               (uint16_t)written, HAL_MAX_DELAY);
    }
}
