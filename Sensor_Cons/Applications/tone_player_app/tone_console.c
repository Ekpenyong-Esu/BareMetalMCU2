/**
 * @file tone_console.c
 * @brief Says over the serial port what the tone player is doing
 */

#include "tone_console.h"

#include "uart_blocking.h"
#include "uart_config.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* One handle per link, static so the driver's pointer stays valid for the
   whole program rather than pointing into a dead stack frame. */
static UART_HandleTypeDef s_halHandle;
static UART_Handle_t s_uart;

/** Cleared when the port would not open, which turns every report into a no-op. */
static bool s_portOpen = false;

/** @brief Put one string on the wire, or nothing if there is no port */
static void ToneConsole_Send(const char *text)
{
    if (!s_portOpen || text == NULL) {
        return;
    }

    UART_Blocking_Transmit(&s_uart, (const uint8_t *)text,
                           (uint16_t)strlen(text), UART_TIMEOUT);
}

void ToneConsole_Init(void)
{
    const UART_Config_t config = {
        .instance   = USART1,
        .baudRate   = UART_DEFAULT_BAUDRATE,
        .wordLength = UART_DEFAULT_WORDLENGTH,
        .stopBits   = UART_DEFAULT_STOPBITS,
        .parity     = UART_DEFAULT_PARITY,
        .mode       = UART_MODE_BLOCKING,
    };

    s_uart.huart = &s_halHandle;
    s_portOpen = (UART_Blocking_Init(&s_uart, &config) == UART_OK);
}

void ToneConsole_ReportReady(const char *outputDescription)
{
    ToneConsole_Send("\r\nTone player started.\r\n");
    ToneConsole_Send("Output: ");
    ToneConsole_Send(outputDescription);
    ToneConsole_Send("\r\n");
}

void ToneConsole_ReportMelody(const Melody_t *melody)
{
    if (melody == NULL) {
        return;
    }

    ToneConsole_Send("Now playing: ");
    ToneConsole_Send(melody->title);
    ToneConsole_Send("\r\n");
}

void ToneConsole_ReportError(const char *reason)
{
    ToneConsole_Send("\r\nTone player stopped: ");
    ToneConsole_Send(reason);
    ToneConsole_Send("\r\n");
}
