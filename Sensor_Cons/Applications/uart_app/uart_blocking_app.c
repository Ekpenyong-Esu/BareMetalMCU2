/**
 * @file uart_blocking_app.c
 * @brief The UART application: blocking-mode echo example
 *
 * Sets up USART1 in blocking mode (115200 8N1) and echoes back each line the
 * user types. Blocking mode is the simplest way to use UART: every call just
 * waits until it is done, so there are no interrupts or callbacks to follow.
 *
 * The catch is that nothing listens while the CPU transmits. At 115200 a
 * 76-byte reply takes 6.6ms, which is about 76 further bytes of arriving data
 * -- and with no interrupt and no ring buffer, every one of them is lost. So
 * this app never transmits mid-message: it accumulates until the terminator
 * arrives and replies once, which keeps the only silent window down to the
 * sub-microsecond gap between two Receive() calls.
 *
 * That restriction is the whole lesson of blocking mode. The interrupt and DMA
 * branches do not need it, because reception there continues in the background.
 */

#include "uart_blocking_app.h"

#include "uart_blocking.h"
#include "uart_config.h"
#include <printf/printf.h>
#include <stdint.h>
#include <stdio.h>

#define UART_BLOCKING_APP_RX_SIZE    64  /* bytes taken from one Receive() call */
#define UART_BLOCKING_APP_LINE_SIZE  256 /* longest line echoed back in one piece */
#define UART_BLOCKING_APP_TX_SIZE    288 /* holds the prefix, the echo and the newline */

/* The driver needs one handle per UART link. Both are static so they live for
 * the whole program instead of on the stack. */
static UART_HandleTypeDef s_halHandle;
static UART_Handle_t s_uart;

/* The line being typed, still incomplete until a newline shows up. */
static uint8_t s_line[UART_BLOCKING_APP_LINE_SIZE];
static uint16_t s_lineLength;
static uint8_t s_txLine[UART_BLOCKING_APP_TX_SIZE];

/* One reply, one transmit: a shorter send means a shorter window in which
 * arriving bytes are lost. The %.*s precision bounds the read, which matters
 * because s_line is not a string. */
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

/* Feed one chunk into the line buffer. Returns true once the line is complete. */
static bool AccumulateLine(const uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = data[i];

        if (byte == '\n') {
            /* Anything after the terminator belongs to the next line, but
             * replying is about to lose those bytes anyway, so drop them. */
            return true;
        }

        if (byte == '\r') {
            continue; /* CRLF terminators: the newline is what ends the line */
        }

        /* A line longer than the buffer keeps its first LINE_SIZE bytes. */
        if (s_lineLength < sizeof(s_line)) {
            s_line[s_lineLength++] = byte;
        }
    }

    return false;
}

void UartBlockingApp_Run(void)
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

    if (UART_Blocking_Init(&s_uart, &config) != UART_OK) {
        return; /* nothing to talk over; the caller decides what to do */
    }

    const char greeting[] = "UART ready. Type something and press enter.\r\n";
    UART_Blocking_Transmit(&s_uart, (const uint8_t *)greeting,
                           (uint16_t)(sizeof(greeting) - 1), UART_TIMEOUT);

    uint8_t rxData[UART_BLOCKING_APP_RX_SIZE];

    for (;;) {
        uint16_t received = 0;

        UART_Status_t status = UART_Blocking_Receive(&s_uart, rxData, sizeof(rxData),
                                                     &received, UART_TIMEOUT);

        if (status == UART_ERROR) {
            /* Bytes were lost, so the line in hand is already incomplete.
             * Reporting it would transmit, losing more; drop it silently and
             * let the next line start clean. */
            s_lineLength = 0;
            continue;
        }

        if (received == 0) {
            continue; /* idle timeout: nobody typed anything */
        }

        if (!AccumulateLine(rxData, received)) {
            continue; /* still mid-line: stay silent so nothing is missed */
        }

        UART_Blocking_Transmit(&s_uart, s_txLine, BuildReply(s_line, s_lineLength), UART_TIMEOUT);
        s_lineLength = 0;
    }
}
