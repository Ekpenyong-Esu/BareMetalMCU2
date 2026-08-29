/**
 * @file dac.h
 * @brief DAC driver aggregator
 *
 * Pulls in the public DAC modules:
 *  - dac_types.h   : data types and constants
 *  - dac_core.h    : initialization and lifecycle
 *  - dac_output.h  : conversion control and output value access
 *  - dac_convert.h : conversion between codes and volts
 *  - dac_dma.h     : streaming a sample buffer over DMA
 *
 * @note Only DAC_CHANNEL_1 is supported. The MSP wires PA4 (DAC_OUT1) alone,
 *       so a channel 2 request is rejected rather than silently driving PA5.
 */

#ifndef DAC_H
#define DAC_H

#include "dac_types.h"
#include "dac_core.h"
#include "dac_output.h"
#include "dac_convert.h"
#include "dac_dma.h"

#endif /* DAC_H */
