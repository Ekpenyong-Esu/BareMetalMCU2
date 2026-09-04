/**
 * @file dac.h
 * @brief DAC driver aggregator
 *
 * Pulls in the public DAC modules:
 *  - dac_types.h   : data types and constants (config, handle, max values)
 *  - dac_core.h    : initialization and lifecycle (Init, DeInit, IsReady)
 *  - dac_output.h  : conversion control and output value access
 *  - dac_convert.h : conversion between codes and volts
 *
 * STM32F4 DAC:
 * - Single DAC peripheral with 2 channels (DAC_OUT1 on PA4, DAC_OUT2 on PA5)
 * - Output buffer: enabled (default, drives loads) or disabled (high-Z)
 * - Triggers: software, timer TRGO (TIM2,4,5,6,7,8), EXTI9
 * - Alignment: 12-bit right/left, 8-bit right
 * - Reference: VDDA (3.3V nominal)
 *
 * The application supplies the channel and the pin that carries it in
 * DAC_ConfigTypeDef (channel, outPort, outPin); the driver assumes no wiring.
 */

#ifndef DAC_H
#define DAC_H

#include "dac_types.h"
#include "dac_core.h"
#include "dac_output.h"
#include "dac_convert.h"

#endif /* DAC_H */
