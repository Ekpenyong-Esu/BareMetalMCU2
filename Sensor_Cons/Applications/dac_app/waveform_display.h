/**
 * @file waveform_display.h
 * @brief Formats DAC waveform samples and prints them over USART1
 */

#ifndef WAVEFORM_DISPLAY_H
#define WAVEFORM_DISPLAY_H

#include "uart.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    UART_Handle_t      uart;
    UART_HandleTypeDef halUart;
} Waveform_Display_t;

/** Bring up USART1 (115200 8N1) and print the banner. */
bool Waveform_DisplayInit(Waveform_Display_t *display);

/** Print one sample as "s[i] = code (x.xxx V)". */
void Waveform_DisplayShow(Waveform_Display_t *display, uint32_t index, uint32_t code);

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_DISPLAY_H */
