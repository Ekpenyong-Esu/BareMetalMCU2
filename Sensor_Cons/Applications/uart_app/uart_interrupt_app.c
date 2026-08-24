/**
 * @file uart_interrupt_app.c
 * @brief The UART application: interrupt-mode echo example
 *
 * Sets up USART1 in interrupt mode (115200 8N1) and echoes back whatever the
 * user types. The loop below never waits on the UART: reception runs in the
 * background and every call returns straight away, so real work can sit
 * alongside the echo instead of behind it.
 */

#include "uart_interrupt_app.h"

#include "uart_config.h"
#include "uart_interrupt.h"
#include <printf/printf.h>
#include <stdint.h>
#include <stdio.h>

#define UART_INTERRUPT_APP_RX_SIZE  64 /* landing area the ISR receives into */
#define UART_INTERRUPT_APP_TX_SIZE  96 /* holds the prefix, the echo and the newline */

static const char kGreeting[] = "UART ready (interrupt mode). Type something and press enter.\r\n";

/* All static: the ISR reads s_txLine and writes s_rxLanding long after Run()
 * has moved past the call that handed them over. */
static UART_HandleTypeDef s_halHandle;
static UART_Handle_t s_uart;
static uint8_t s_rxLanding[UART_INTERRUPT_APP_RX_SIZE];
static uint8_t s_txLine[UART_INTERRUPT_APP_TX_SIZE];

/* One reply, one Write(): the driver sends a single buffer at a time, so the
 * whole line is assembled before any of it goes out. The %.*s precision bounds
 * the read, which matters because echo is not a string. */
static uint16_t BuildReply(const uint8_t *echo, uint16_t echoLength)
{
    int written = snprintf((char *)s_txLine, sizeof(s_txLine), "You said: %.*s\r\n",
                           (int)echoLength, (const char *)echo);

    if (written < 0) {
        return 0;
    }

    /* snprintf reports what a big enough buffer would have taken. */
    return (written < (int)sizeof(s_txLine)) ? (uint16_t)written : (uint16_t)sizeof(s_txLine);
}

void UartInterruptApp_Run(void)
{
    const UART_Config_t config = {
        .instance   = USART1,
        .baudRate   = UART_DEFAULT_BAUDRATE,
        .wordLength = UART_DEFAULT_WORDLENGTH,
        .stopBits   = UART_DEFAULT_STOPBITS,
        .parity     = UART_DEFAULT_PARITY,
        .mode       = UART_MODE_INTERRUPT,
    };

    s_uart.huart = &s_halHandle;

    if (UART_Interrupt_Init(&s_uart, &config, s_rxLanding, sizeof(s_rxLanding)) != UART_OK) {
        return; /* nothing to talk over; the caller decides what to do */
    }

    /* kGreeting is const and lives forever, so the ISR can send it at leisure. */
    UART_Interrupt_Write(&s_uart, (const uint8_t *)kGreeting, (uint16_t)(sizeof(kGreeting) - 1));

    uint8_t rxData[UART_INTERRUPT_APP_RX_SIZE];

    for (;;) {
        /* Other work belongs here. Bytes keep arriving either way. */

        if (!UART_Interrupt_IsTxDone(&s_uart)) {
            continue; /* previous reply still going out; leave s_txLine alone */
        }

        uint16_t received = 0;

        if (UART_Interrupt_Read(&s_uart, rxData, sizeof(rxData), &received) != UART_OK) {
            continue;
        }

        if (received == 0) {
            continue; /* quiet line, not an error */
        }

        /* Echo exactly the bytes that arrived. rxData is not a string, so the
         * length has to come from the driver rather than from strlen(). */
        UART_Interrupt_Write(&s_uart, s_txLine, BuildReply(rxData, received));
    }
}
