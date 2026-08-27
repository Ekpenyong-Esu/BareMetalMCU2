/**
 * @file adc_voltmeter_app.c
 * @brief The ADC application: voltmeter example
 *
 * Measures the voltage on PA0 (ADC1 channel 0) and prints it over USART1
 * (115200 8N1) once per second. Wire a potentiometer or any 0-3.3 V source to
 * PA0 and watch the value change in the terminal.
 *
 * The ADC is read in polling mode: each pass of the loop asks for one
 * conversion and waits for it. A conversion takes microseconds, so blocking
 * here costs nothing -- unlike the UART echo app, there is no background
 * traffic to miss while waiting.
 *
 * The raw ADC count (0-4095 for 12-bit) means nothing on its own, so the
 * driver scales it against the 3.3 V reference before this file ever sees it.
 */

#include "adc_voltmeter_app.h"

#include "adc_convert.h"
#include "adc_core.h"
#include "adc_measure.h"
#include "uart_blocking.h"
#include "uart_config.h"
#include <printf/printf.h>
#include <stdint.h>
#include <stdio.h>

#define ADC_VOLTMETER_APP_CHANNEL    ADC_CHANNEL_0  /* PA0 on the discovery header */
#define ADC_VOLTMETER_APP_PERIOD_MS  1000U          /* one reading per second */
#define ADC_VOLTMETER_APP_TX_SIZE    64             /* one line: "Voltage: x.xxx V\r\n" */

static const char kGreeting[] = "ADC voltmeter ready. Reading PA0 once per second.\r\n";

/* All static: the UART and ADC drivers hold pointers to these handles for the
 * whole run, so they must outlive the call that set them up. */
static UART_HandleTypeDef s_halUart;
static UART_Handle_t s_uart;
static ADC_HandleStruct s_adc;


static char s_txLine[ADC_VOLTMETER_APP_TX_SIZE];

void AdcVoltmeterApp_Run(void)
{
    const UART_Config_t uartConfig = {
        .instance   = USART1,
        .baudRate   = UART_DEFAULT_BAUDRATE,
        .wordLength = UART_DEFAULT_WORDLENGTH,
        .stopBits   = UART_DEFAULT_STOPBITS,
        .parity     = UART_DEFAULT_PARITY,
        .mode       = UART_MODE_BLOCKING,
    };

    const ADC_ConfigTypeDef adcConfig = {
        .instance      = ADC1,
        .channel       = ADC_VOLTMETER_APP_CHANNEL,
        .resolution    = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_56CYCLES,
        .conv_mode     = ADC_MODE_SINGLE,
        .dma_enabled   = false,
    };

    s_uart.huart = &s_halUart;

    if (UART_Blocking_Init(&s_uart, &uartConfig) != UART_OK) {
        return; /* no way to report readings; the caller decides what to do */
    }

    UART_Blocking_Transmit(&s_uart, (const uint8_t *)kGreeting,
                           (uint16_t)(sizeof(kGreeting) - 1), HAL_MAX_DELAY);

    if (ADC_Init(&s_adc, &adcConfig) != HAL_OK) {
        return; /* nothing to measure with */
    }

    for (;;) {
        float voltage = ADC_ReadChannelVoltage(&s_adc, ADC_VOLTMETER_APP_CHANNEL);

        if (voltage >= 0.0f) {
            /* %d.%03d keeps float formatting out of the printf library. */
            int millivolts = (int)(voltage * 1000.0f);
            int written = snprintf(s_txLine, sizeof(s_txLine), "Voltage: %d.%03d V\r\n",
                                   millivolts / 1000, millivolts % 1000);

            if (written > 0) {
                UART_Blocking_Transmit(&s_uart, (const uint8_t *)s_txLine,
                                       (uint16_t)written, HAL_MAX_DELAY);
            }
        }

        HAL_Delay(ADC_VOLTMETER_APP_PERIOD_MS);
    }
}
