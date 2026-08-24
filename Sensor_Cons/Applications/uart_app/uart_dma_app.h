/**
 * @file uart_dma_app.h
 * @brief The UART application: DMA-mode echo example
 */

#ifndef UART_DMA_APP_H
#define UART_DMA_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Set up USART1 in DMA mode and echo back whatever the user types. */
void UartDmaApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_DMA_APP_H */
