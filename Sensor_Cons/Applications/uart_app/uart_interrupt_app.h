/**
 * @file uart_interrupt_app.h
 * @brief The UART application: interrupt-mode echo example
 */

#ifndef UART_INTERRUPT_APP_H
#define UART_INTERRUPT_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Set up USART1 in interrupt mode and echo back whatever the user types. */
void UartInterruptApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_INTERRUPT_APP_H */
