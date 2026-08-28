/**
 * @file adc_voltmeter_app.c
 * @brief The ADC application: voltmeter example, bare-metal super-loop version.
 *
 * No FreeRTOS here: a single blocking loop samples PA0 (ADC1 channel 0) once
 * a second and prints each reading over USART1 (115200 8N1) before sleeping.
 *
 * Flow:  ADC -> read -> format -> UART -> HAL_Delay
 * This is the simpler cousin of the FreeRTOS queue-based version: the UART
 * transmit blocks the loop, but at 1 sample/s nobody cares, and the code
 * stays small enough to see the whole application on one screen.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   adc_voltmeter_app   -> composition only (this module)
 *   voltage_reader      -> ADC setup + single-channel voltage reads
 *   voltage_display     -> UART setup + formatted voltage output
 */

#include "adc_voltmeter_app.h"

#include "main.h"
#include "voltage_display.h"
#include "voltage_reader.h"

/* Sample cadence (ms): how often the loop takes a reading. */
#define SAMPLE_PERIOD_MS      1000u

/* Handles --------------------------------------------------------------- */
static VoltageReader_t  s_reader;
static VoltageDisplay_t s_display;

/* ----------------------------------------------------------------------- */
void AdcVoltmeterApp_Run(void)
{
    if (!VoltageDisplay_Init(&s_display)) {
        Error_Handler(); /* no way to report readings; nothing else to do */
    }

    if (!VoltageReader_Init(&s_reader)) {
        Error_Handler(); /* nothing to measure with */
    }

    /* Super-loop: sample, print, sleep. Never returns. */
    for (;;) {
        float voltage = VoltageReader_Read(&s_reader);

        VoltageDisplay_Show(&s_display, voltage);
        HAL_Delay(SAMPLE_PERIOD_MS);
    }
}
