/**
 * @file uart_blocking_app.h
 * @brief The UART application: blocking-mode echo example
 */

#ifndef UART_BLOCKING_APP_H
#define UART_BLOCKING_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Set up USART1 in blocking mode and echo back whatever the user types. */
void UartBlockingApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_APP_H */
