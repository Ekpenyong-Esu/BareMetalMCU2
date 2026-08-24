/**
 * @file uart_blocking_app.c
 * @brief The UART application: blocking-mode echo example
 *
 * Sets up USART1 in blocking mode (115200 8N1) and echoes back whatever the
 * user types. Blocking mode is the simplest way to use UART: every call just
 * waits until it is done, so there are no interrupts or callbacks to follow.
 * The cost is that bytes arriving while the CPU is transmitting are lost.
 */

#include "uart_blocking_app.h"

#include "uart_blocking.h"
#include "uart_config.h"
#include <stdint.h>
#include <string.h>

#define UART_BLOCKING_APP_RX_SIZE 64 /* max bytes taken in one read */

/* The driver needs one handle per UART link. Both are static so they live for
 * the whole program instead of on the stack. */
static UART_HandleTypeDef s_halHandle;
static UART_Handle_t s_uart;

/* Small helper so every send below is one line instead of three. */
static void SendText(const char *text)
{
    UART_Blocking_Transmit(&s_uart, (const uint8_t *)text, (uint16_t)strlen(text), UART_TIMEOUT);
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

    SendText("UART ready. Type something and press enter.\r\n");

    uint8_t rxData[UART_BLOCKING_APP_RX_SIZE];

    for (;;) {
        uint16_t received = 0;

        UART_Status_t status = UART_Blocking_Receive(&s_uart, rxData, sizeof(rxData), &received, UART_TIMEOUT);

        if (status == UART_ERROR) {
            SendText("RX error\r\n"); /* driver already cleared the flags */
            continue;
        }

        if (received == 0) {
            continue; /* idle timeout: nobody typed anything */
        }

        /* Echo exactly the bytes that arrived. rxData is not a string, so the
         * length has to come from the driver rather than from strlen(). */
        SendText("You said: ");
        UART_Blocking_Transmit(&s_uart, rxData, received, UART_TIMEOUT);
        SendText("\r\n");
    }
}
