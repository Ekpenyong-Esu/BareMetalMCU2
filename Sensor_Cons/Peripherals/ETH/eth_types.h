/**
 * @file eth_types.h
 * @brief Data types and constants for the Ethernet driver
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
 * @brief Ethernet configuration
 */
typedef struct {
    uint8_t macAddr[ETH_ADDR_LEN];   /**< Station MAC address */
    uint32_t speed;                  /**< ETH_SPEED_10M or ETH_SPEED_100M */
    uint32_t duplexMode;             /**< ETH_FULLDUPLEX_MODE or ETH_HALFDUPLEX_MODE */
    uint32_t mediaInterface;         /**< ETH_MEDIA_INTERFACE_RMII or ETH_MEDIA_INTERFACE_MII */
} ETH_Config_t;

/**
 * @brief Ethernet handle
 */
typedef struct {
    ETH_HandleTypeDef heth;          /**< HAL ETH handle */
    ETH_Config_t config;             /**< Configuration in force */
    bool initialized;                /**< Initialization status */
} ETH_Handle_t;

/**
 * @brief Parsed Ethernet frame
 */
typedef struct {
    uint8_t destination[ETH_ADDR_LEN];  /**< Destination MAC address */
    uint8_t source[ETH_ADDR_LEN];       /**< Source MAC address */
    uint16_t type;                      /**< EtherType in host order */
    uint8_t *payload;                   /**< Payload data */
    uint32_t payloadLength;             /**< Payload length in bytes */
} ETH_Frame_t;

#ifdef __cplusplus
}
#endif

#endif /* ETH_TYPES_H */
