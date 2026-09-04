/**
 * @file servo_console.c
 * @brief Says over the serial port what the servo sweep is doing
 *
 * Blocking UART only: every report is a few dozen bytes at most and the sweep
 * pauses for the servo to settle anyway, so there is nothing to gain from DMA.
 */

#include "servo_console.h"

#include "uart_blocking.h"
#include "uart_config.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define SERVO_CONSOLE_LINE_SIZE 32U

static UART_HandleTypeDef s_halHandle;
static UART_Handle_t s_uart;

/** Cleared when the port would not open, which turns every report into a no-op. */
static bool s_portOpen = false;

/** @brief Put one string on the wire, or nothing if there is no port */
static void ServoConsole_Send(const char *text) {
    if (!s_portOpen || text == NULL) {
        return;
    }

    UART_Blocking_Transmit(&s_uart, (const uint8_t *)text, (uint16_t)strlen(text), UART_TIMEOUT);
}

void ServoConsole_Init(USART_TypeDef *instance) {
    const UART_Config_t config = {
        .instance = instance,
        .txPort = GPIOA,
        .txPin = GPIO_PIN_9,
        .rxPort = GPIOA,
        .rxPin = GPIO_PIN_10,
        .baudRate = UART_DEFAULT_BAUDRATE,
        .wordLength = UART_DEFAULT_WORDLENGTH,
        .stopBits = UART_DEFAULT_STOPBITS,
        .parity = UART_DEFAULT_PARITY,
        .mode = UART_MODE_BLOCKING,
    };

    s_uart.huart = &s_halHandle;
    s_portOpen = (UART_Blocking_Init(&s_uart, &config) == UART_OK);
}

void ServoConsole_ReportReady(const char *outputDescription) {
    ServoConsole_Send("\r\nServo sweep started.\r\n");
    ServoConsole_Send("Output: ");
    ServoConsole_Send(outputDescription);
    ServoConsole_Send("\r\n");
}

void ServoConsole_ReportAngle(uint16_t angleDeg) {
    char line[SERVO_CONSOLE_LINE_SIZE];

    snprintf(line, sizeof(line), "angle: %u deg\r\n", (unsigned)angleDeg);
    ServoConsole_Send(line);
}

void ServoConsole_ReportError(const char *reason) {
    ServoConsole_Send("\r\nServo sweep stopped: ");
    ServoConsole_Send(reason);
    ServoConsole_Send("\r\n");
}
