/**
 * @file    eth_types.h
 * @brief   Shared types for the Ethernet driver
 * @details This file holds the basic types and settings for Ethernet.
 *
 * How it works (in simple words):
 * - Ethernet sends data as small packets called frames over a network cable.
 * - Each frame has a sender address, a receiver address, a type, and the data.
 * - The driver sets up the network chip, sends frames, and reads frames that arrive.
 */

#ifndef ETH_TYPES_H
#define ETH_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
#include <stdbool.h>
#include <stdint.h>

/* Ethernet protocol types */
#define ETH_TYPE_ARP             0x0806U
#define ETH_TYPE_IPV4            0x0800U
#define ETH_TYPE_IPV6            0x86DDU

/* Ethernet frame layout */
#define ETH_ADDR_LEN             6U
#define ETH_HEADER_LEN           14U
#define ETH_TYPE_OFFSET          12U
#define ETH_PAYLOAD_OFFSET       14U
#define ETH_MIN_FRAME_LEN        60U     /**< Shortest legal frame excluding the FCS */
#define ETH_MAX_FRAME_LEN        1514U   /**< Header plus the largest untagged payload */
#define ETH_MAX_PAYLOAD_LEN      (ETH_MAX_FRAME_LEN - ETH_HEADER_LEN)

/** @brief NVIC priority applied to the Ethernet global interrupt */
#define ETH_IRQ_PRIORITY         5U
#define ETH_IRQ_SUBPRIORITY      0U

/** @brief Timeout of a blocking transmit, in milliseconds */
#define ETH_TX_TIMEOUT_MS        100U

/**
 * @brief Settings for Ethernet
 * @details Pick the board address (MAC), speed, and cable type.
 */
typedef struct {
    uint8_t macAddr[ETH_ADDR_LEN];   /**< Board address on the network (6 bytes) */
    uint32_t speed;                  /**< Speed: 10M or 100M */
    uint32_t duplexMode;             /**< Send and receive at same time or not */
    uint32_t mediaInterface;         /**< Cable type: RMII or MII */
} ETH_Config_t;

/**
 * @brief Handle that keeps all Ethernet info in one place
 * @details Holds the low-level handle, settings, and if it is ready.
 */
typedef struct {
    ETH_HandleTypeDef heth;          /**< Low-level Ethernet handle */
    ETH_Config_t config;             /**< Settings in use */
    bool initialized;                /**< True if ready to use */
} ETH_Handle_t;

/**
 * @brief One network packet (frame)
 * @details Has who it is for, who sent it, what kind it is, and the data.
 */
typedef struct {
    uint8_t destination[ETH_ADDR_LEN];  /**< Who should receive it */
    uint8_t source[ETH_ADDR_LEN];       /**< Who sent it */
    uint16_t type;                      /**< What kind of data it is */
    uint8_t *payload;                   /**< The actual data */
    uint32_t payloadLength;             /**< How many bytes of data */
} ETH_Frame_t;

#ifdef __cplusplus
}
#endif

#endif /* ETH_TYPES_H */
