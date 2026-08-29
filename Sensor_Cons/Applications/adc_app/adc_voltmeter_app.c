/**
 * @file adc_voltmeter_app.c
 * @brief The ADC application: multi-channel voltmeter, DMA-driven.
 *
 * One ADC, several inputs. A scan sequence walks PA0, PA1, PA2 and the
 * internal reference in rank order, and every conversion raises a DMA request
 * that writes the result straight into memory. The CPU never handles a sample.
 *
 * Flow:  start scan -> ADC -> DMA -> buffer -> (one IRQ) -> print -> HAL_Delay
 *
 * Compared with the interrupt branch this trades four interrupts per scan for
 * one, and the saving grows with the sequence: a sixteen-channel scan still
 * costs exactly one. The buffer lives in the reader because the DMA writes to
 * it directly, so it has to outlive the transfer.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   adc_voltmeter_app   -> composition only (this module)
 *   voltage_reader      -> ADC scan setup + DMA transfer of the results
 *   voltage_display     -> UART setup + formatted voltage output
 */

#include "adc_voltmeter_app.h"

#include "main.h"
#include "voltage_display.h"
#include "voltage_reader.h"

#include <stdint.h>

/* Sample cadence (ms): how often the loop runs a scan. */
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

    /* Super-loop: scan, print, sleep. Never returns. */
    for (;;) {
        float volts[VOLTAGE_READER_CHANNEL_COUNT];

        if (VoltageReader_Start(&s_reader)) {
            /* Idle until the transfer completes. A real workload would go
             * here instead; the scan needs nothing from the CPU. */
            while (!VoltageReader_IsComplete(&s_reader)) {
                __WFI();
            }

            if (VoltageReader_Take(&s_reader, volts)) {
                for (uint32_t i = 0; i < VOLTAGE_READER_CHANNEL_COUNT; i++) {
                    VoltageDisplay_Show(&s_display, VoltageReader_ChannelName(i), volts[i]);
                }
                VoltageDisplay_EndScan(&s_display);
            }
        }

        HAL_Delay(SAMPLE_PERIOD_MS);
    }
}
