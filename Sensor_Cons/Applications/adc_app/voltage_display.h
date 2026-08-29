/**
 * @file voltage_display.h
 * @brief SRP module: formats voltage readings and prints them over USART1.
 */

#ifndef VOLTAGE_DISPLAY_H
#define VOLTAGE_DISPLAY_H

#include "uart_config.h"
#include "uart_types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    UART_HandleTypeDef halUart;
    UART_Handle_t      uart;
} VoltageDisplay_t;

/** Bring up USART1 (115200 8N1) and print the ready-banner. */
bool VoltageDisplay_Init(VoltageDisplay_t *display);

/** Print one "PA0 : x.xxx V" line. Silently ignores negative (error) readings. */
void VoltageDisplay_Show(VoltageDisplay_t *display, const char *label, float voltage);

/** Blank line, so consecutive scans stay visually separated. */
void VoltageDisplay_EndScan(VoltageDisplay_t *display);

#ifdef __cplusplus
}
#endif

#endif /* VOLTAGE_DISPLAY_H */
