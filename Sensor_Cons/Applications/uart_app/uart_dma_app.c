/**
 * @file uart_dma_app.c
 * @brief The UART application: DMA-mode echo example
 *
 * Sets up USART1 in DMA mode (115200 8N1) and echoes back each line the user
 * types. Compare it with the interrupt-mode example: the loop is the same,
 * because the two modes present the same non-blocking API. What changed is
 * underneath -- the DMA controller moves the bytes, so the CPU is not
 * interrupted once per byte.
 *
 * The driver hands over a byte stream with no idea where a message ends, so
 * deciding that is this file's job: bytes are accumulated until a newline
 * arrives, then the whole line goes back at once.
 */

#include "uart_dma_app.h"

#include "uart_config.h"
#include "uart_dma.h"
#include <printf/printf.h>
#include <stdint.h>
#include <stdio.h>

#define UART_DMA_APP_RX_SIZE    256 /* landing area the DMA stream writes into */
#define UART_DMA_APP_LINE_SIZE  256 /* longest line echoed back in one piece */
#define UART_DMA_APP_TX_SIZE    288 /* holds the prefix, the echo and the newline */

static const char kGreeting[] = "UART ready (DMA mode). Type something and press enter.\r\n";

/* All static, so they sit in ordinary SRAM where DMA2 can reach them, and so
 * they outlive the call that hands them to the driver. A buffer on the stack
 * would be gone before the stream finished with it; one in CCMRAM would not be
 * reachable by DMA at all. */
static UART_HandleTypeDef s_halHandle;
static UART_Handle_t s_uart;
static uint8_t s_rxLanding[UART_DMA_APP_RX_SIZE];
static uint8_t s_txLine[UART_DMA_APP_TX_SIZE];

/* The line being typed, still incomplete until a newline shows up. */
static uint8_t s_line[UART_DMA_APP_LINE_SIZE];
static uint16_t s_lineLength;

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

void UartDmaApp_Run(void)
{
    const UART_Config_t config = {
        .instance   = USART1,
        .baudRate   = UART_DEFAULT_BAUDRATE,
        .wordLength = UART_DEFAULT_WORDLENGTH,
        .stopBits   = UART_DEFAULT_STOPBITS,
        .parity     = UART_DEFAULT_PARITY,
        .mode       = UART_MODE_DMA,
    };

    s_uart.huart = &s_halHandle;

    if (UART_DMA_Init(&s_uart, &config, s_rxLanding, sizeof(s_rxLanding)) != UART_OK) {
        return; /* nothing to talk over; the caller decides what to do */
    }

    /* kGreeting is const and lives forever, so the stream can send it at leisure. */
    UART_DMA_Write(&s_uart, (const uint8_t *)kGreeting, (uint16_t)(sizeof(kGreeting) - 1));

    for (;;) {
        /* Other work belongs here. Bytes keep arriving either way. */

        if (!UART_DMA_IsTxDone(&s_uart)) {
            continue; /* previous reply still going out; leave s_txLine alone */
        }

        /* One byte per pass. The ring absorbs the burst, so taking them singly
         * costs nothing and keeps the terminator handling free of bookkeeping
         * about bytes left over after a newline. */
        uint8_t byte = 0;
        uint16_t received = 0;

        if (UART_DMA_Read(&s_uart, &byte, sizeof(byte), &received) != UART_OK) {
            continue;
        }

        if (received == 0) {
            continue; /* quiet line, not an error */
        }

        if (byte == '\n') {
            UART_DMA_Write(&s_uart, s_txLine, BuildReply(s_line, s_lineLength));
            s_lineLength = 0;
            continue;
        }

        if (byte == '\r') {
            continue; /* CRLF terminators: the newline is what ends the line */
        }

        /* A line longer than the buffer keeps its first LINE_SIZE bytes; the
         * overflow is dropped rather than splitting the reply in two. */
        if (s_lineLength < sizeof(s_line)) {
            s_line[s_lineLength++] = byte;
        }
    }
}
