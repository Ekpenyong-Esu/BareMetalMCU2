/**
 * @file voltage_display.c
 * @brief SRP module: formats voltage readings and prints them over USART1.
 */

#include "voltage_display.h"

#include "uart_blocking.h"

#include <stdint.h>
#include <stdio.h>

#define VOLTAGE_DISPLAY_TX_SIZE  64  /* one line: "Voltage: x.xxx V\r\n" */

static const char kGreeting[] = "ADC voltmeter ready. Reading PA0 once per second.\r\n";

static char s_txLine[VOLTAGE_DISPLAY_TX_SIZE];

bool VoltageDisplay_Init(VoltageDisplay_t *display)
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

void VoltageDisplay_Show(VoltageDisplay_t *display, float voltage)
{
    if (voltage < 0.0f) {
        return; /* reader signals "no sample" with a negative value */
    }

    /* %d.%03d keeps float formatting out of the printf library. */
    int millivolts = (int)(voltage * 1000.0f);
    int written = snprintf(s_txLine, sizeof(s_txLine), "Voltage: %d.%03d V\r\n",
                           millivolts / 1000, millivolts % 1000);

    if (written > 0) {
        UART_Blocking_Transmit(&display->uart, (const uint8_t *)s_txLine,
                               (uint16_t)written, HAL_MAX_DELAY);
    }
}
