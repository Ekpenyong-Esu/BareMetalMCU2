/**
 * @file    eeprom_types.h
 * @brief   EEPROM vocabulary: status codes, device types, configuration, handle
 * @details Pure data definitions shared by every EEPROM module. Contains no
 *          behaviour, so module headers can include it without cycles.
 */

#ifndef __EEPROM_TYPES_H__
#define __EEPROM_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "i2c_types.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief EEPROM Status enumeration
 */
typedef enum {
    EEPROM_OK = 0,          /**< Operation completed successfully */
    EEPROM_ERROR,           /**< General error occurred */
    EEPROM_TIMEOUT,         /**< Operation timed out */
    EEPROM_BUSY,            /**< EEPROM is busy (internal write cycle) */
    EEPROM_INVALID_PARAM,   /**< Invalid parameter provided */
    EEPROM_INVALID_ADDRESS, /**< Invalid memory address */
    EEPROM_NOT_INITIALIZED  /**< Driver not initialized */
} EEPROM_StatusTypeDef;

/**
 * @brief EEPROM device type enumeration
 */
typedef enum {
    EEPROM_TYPE_M24LR64 = 0, /**< M24LR64 - 64Kbit (8KB) EEPROM */
    EEPROM_TYPE_M24C01,      /**< M24C01 - 1Kbit (128B) EEPROM */
    EEPROM_TYPE_M24C02,      /**< M24C02 - 2Kbit (256B) EEPROM */
    EEPROM_TYPE_M24C04,      /**< M24C04 - 4Kbit (512B) EEPROM */
    EEPROM_TYPE_M24C08,      /**< M24C08 - 8Kbit (1KB) EEPROM */
    EEPROM_TYPE_M24C16,      /**< M24C16 - 16Kbit (2KB) EEPROM */
    EEPROM_TYPE_M24C32,      /**< M24C32 - 32Kbit (4KB) EEPROM */
    EEPROM_TYPE_M24C64,      /**< M24C64 - 64Kbit (8KB) EEPROM */
    EEPROM_TYPE_M24C128,     /**< M24C128 - 128Kbit (16KB) EEPROM */
    EEPROM_TYPE_M24C256,     /**< M24C256 - 256Kbit (32KB) EEPROM */
    EEPROM_TYPE_M24C512,     /**< M24C512 - 512Kbit (64KB) EEPROM */
    EEPROM_TYPE_AT24C256,    /**< AT24C256 - 256Kbit (32KB) EEPROM */
    EEPROM_TYPE_CUSTOM       /**< Custom EEPROM configuration */
} EEPROM_TypeDef;

/**
 * @brief EEPROM configuration structure
 */
typedef struct {
    uint8_t i2cAddress;    /**< I2C device address (7-bit) */
    uint8_t i2cAddressAlt; /**< Alternative I2C address (for dual-address chips) */
    uint32_t totalSize;    /**< Total EEPROM size in bytes */
    uint16_t pageSize;     /**< Page size for write operations */
    uint8_t addressSize;   /**< Memory address size (1 or 2 bytes) */
} EEPROM_ConfigTypeDef;

/**
 * @brief EEPROM handle structure
 * @note  The bus is opened by the application and handed to EEPROM_Init*;
 *        the driver only registers its own device record on it.
 */
typedef struct {
    I2C_Bus_t *bus;              /**< Bus the chip is wired to, owned by the application */
    I2C_Device_t device;         /**< This chip's record on that bus */
    EEPROM_ConfigTypeDef config; /**< EEPROM configuration */
    EEPROM_TypeDef type;         /**< EEPROM type */
    uint8_t activeAddress;       /**< Currently active I2C address */
    bool initialized;            /**< Initialization flag */
} EEPROM_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief Default I2C addresses for M24LR64
 */
#define EEPROM_I2C_ADDRESS_A01 0x50U /**< Primary address (A0=1) - 7-bit */
#define EEPROM_I2C_ADDRESS_A02 0x53U /**< Secondary address (A0=2) - 7-bit */

/**
 * @brief Default page sizes
 */
#define EEPROM_PAGESIZE_4 4U     /**< 4-byte page (M24LR64) */
#define EEPROM_PAGESIZE_8 8U     /**< 8-byte page */
#define EEPROM_PAGESIZE_16 16U   /**< 16-byte page */
#define EEPROM_PAGESIZE_32 32U   /**< 32-byte page */
#define EEPROM_PAGESIZE_64 64U   /**< 64-byte page */
#define EEPROM_PAGESIZE_128 128U /**< 128-byte page */

/**
 * @brief EEPROM sizes
 */
#define EEPROM_SIZE_128B 128U   /**< 1Kbit */
#define EEPROM_SIZE_256B 256U   /**< 2Kbit */
#define EEPROM_SIZE_512B 512U   /**< 4Kbit */
#define EEPROM_SIZE_1KB 1024U   /**< 8Kbit */
#define EEPROM_SIZE_2KB 2048U   /**< 16Kbit */
#define EEPROM_SIZE_4KB 4096U   /**< 32Kbit */
#define EEPROM_SIZE_8KB 8192U   /**< 64Kbit (M24LR64) */
#define EEPROM_SIZE_16KB 16384U /**< 128Kbit */
#define EEPROM_SIZE_32KB 32768U /**< 256Kbit */
#define EEPROM_SIZE_64KB 65536U /**< 512Kbit */

/**
 * @brief Timeout values
 */
#define EEPROM_TIMEOUT_DEFAULT 1000U /**< Default timeout in ms */
#define EEPROM_WRITE_CYCLE_TIME 5U   /**< Typical write cycle time in ms */
#define EEPROM_MAX_TRIALS 300U       /**< Max trials for device ready */

/**
 * @brief Stack buffer size used by the bulk erase/verify helpers
 */
#define EEPROM_CHUNK_SIZE 64U

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_TYPES_H__ */
