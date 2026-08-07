/**
 * @file eth.h
 * @brief Ethernet driver aggregator
 *
 * Pulls in the public Ethernet modules:
 *  - eth_types.h : data types and constants
 *  - eth_core.h  : initialization, lifecycle and MAC configuration
 *  - eth_frame.h : frame transmission and reception
 *  - eth_irq.h   : interrupt control and event callbacks
 *
 * eth_buffers.h holds the internal DMA descriptor and buffer ownership and is
 * deliberately not exposed here.
 *
 * @note The STM32F429I-DISC1 does not carry an Ethernet PHY. Using this driver
 *       requires a board that provides one along with a matching
 *       HAL_ETH_MspInit() that configures the RMII or MII pins and clocks.
 */

#ifndef ETH_H
#define ETH_H

#include "eth_types.h"
#include "eth_core.h"
#include "eth_frame.h"
#include "eth_irq.h"

#endif /* ETH_H */
