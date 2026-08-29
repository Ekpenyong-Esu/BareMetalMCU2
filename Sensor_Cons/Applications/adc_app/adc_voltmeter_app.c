/**
 * @file adc_voltmeter_app.c
 * @brief The ADC application: multi-channel voltmeter, polled super-loop.
 *
 * One ADC, several inputs. A scan sequence walks PA0, PA1, PA2 and the
 * internal reference in rank order, and the loop waits for each conversion in
 * turn before reading it out.
 *
 * Flow:  start scan -> wait, read -> wait, read -> ... -> print -> HAL_Delay
 *
 * The waiting is the point of this version: the CPU spins inside
 * ADC_PollForConversion for the whole sequence and can do nothing else. That
 * is affordable at one scan per second, and it makes the sequencing visible
 * with no callbacks to follow. The interrupt and DMA branches remove those
 * waits one at a time.
 *
 * Layering:
 *   main.c              -> chooses and runs the application
 *   adc_voltmeter_app   -> composition only (this module)
 *   voltage_reader      -> ADC scan setup + polled voltage reads
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

        if (VoltageReader_Read(&s_reader, volts)) {
            for (uint32_t i = 0; i < VOLTAGE_READER_CHANNEL_COUNT; i++) {
                VoltageDisplay_Show(&s_display, VoltageReader_ChannelName(i), volts[i]);
            }
            VoltageDisplay_EndScan(&s_display);
        }

        HAL_Delay(SAMPLE_PERIOD_MS);
    }
}
