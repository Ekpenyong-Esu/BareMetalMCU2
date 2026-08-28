/**
 * @file adc.h
 * @brief STM32F429I-DISC1 ADC Driver - public interface aggregator
 *
 * This header is the single public entry point for all ADC features.
 * Each feature is implemented as its own single-responsibility module:
 * - adc_types.h    Data types, constants and the shared entry guard
 * - adc_clock.h    Clock enable and rate query
 * - adc_core.h     Initialization, channel selection and the handle registry
 * - adc_channels.h Channel to pin mapping and board availability
 * - adc_convert.h  Conversion control: polling, interrupt and DMA
 * - adc_measure.h  Raw count to volts, and the on-die sensors
 * - adc_hw.h       DMA routing and parameter validation
 *
 * Consumers may include adc.h (everything) or only the module header
 * they need.
 *
 * @note ADC_Init() enables the clock, configures the pin and selects the
 *       requested channel, so nothing has to be turned on beforehand.
 *
 * @note On the STM32F429I-DISC1 almost every analog-capable pin is already
 *       driven by the TFT, the SDRAM, the gyroscope or the USB power switch.
 *       Only PA5 (IN5), PC3 (IN13) and PF6 (ADC3_IN4) are free; check with
 *       ADC_IsChannelFreeOnBoard() before wiring a sensor.
 */

#ifndef ADC_H
#define ADC_H

/* Public interface aggregator: each module header is self-contained and brings
 * in the types it needs. Include adc.h for everything, or a single module
 * header (e.g. adc_measure.h) for minimal dependencies. */
#include "adc_types.h"
#include "adc_clock.h"
#include "adc_core.h"
#include "adc_channels.h"
#include "adc_convert.h"
#include "adc_measure.h"
#include "adc_hw.h"

#endif /* ADC_H */
