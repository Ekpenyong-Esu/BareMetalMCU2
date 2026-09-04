/**
 ******************************************************************************
 * @file    flash_types.h
 * @brief   Internal Flash memory data types and constants
 * @details Status codes, sector descriptors and the STM32F429 Flash memory
 *          map. Contains no behaviour.
 ******************************************************************************
 */

#ifndef __FLASH_TYPES_H__
#define __FLASH_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief FLASH Status enumeration
 * @note  Prefixed with FLASH_STATUS_ to avoid conflicts with HAL defines
 */
typedef enum {
    FLASH_STATUS_OK = 0,          /**< Operation completed successfully */
    FLASH_STATUS_ERROR,           /**< General error occurred */
    FLASH_STATUS_ERROR_PROGRAM,   /**< Programming error */
    FLASH_STATUS_ERROR_WRP,       /**< Write protection error */
    FLASH_STATUS_ERROR_OP,        /**< Operation error */
    FLASH_STATUS_INVALID_PARAM,   /**< Invalid parameter provided */
    FLASH_STATUS_INVALID_ADDRESS, /**< Invalid address */
    FLASH_STATUS_BUSY             /**< Flash is busy */
} FLASH_StatusTypeDef;

/**
 * @brief Flash sector information structure
 */
typedef struct {
    uint32_t SectorNumber; /**< Sector number (0-23 for 2MB) */
    uint32_t StartAddress; /**< Sector start address */
    uint32_t Size;         /**< Sector size in bytes */
} FLASH_SectorInfoTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup FLASH_Constants FLASH Driver Constants
 * @{
 */

/**
 * @brief STM32F429 Flash memory map
 * @note  Total Flash: 2MB (Bank 1: 1MB, Bank 2: 1MB)
 */
#define FLASH_BASE_ADDRESS 0x08000000U /**< Flash base address */
#define FLASH_END_ADDRESS 0x081FFFFFU  /**< Flash end address (2MB) */
#define FLASH_BANK1_END 0x080FFFFFU    /**< Bank 1 end address */
#define FLASH_BANK2_BASE 0x08100000U   /**< Bank 2 base address */

/**
 * @brief Flash sector sizes for STM32F429
 * @note  Bank 1 and Bank 2 have identical layout
 */
#define FLASH_SECTOR_SIZE_16KB 0x4000U   /**< 16 KB sector */
#define FLASH_SECTOR_SIZE_64KB 0x10000U  /**< 64 KB sector */
#define FLASH_SECTOR_SIZE_128KB 0x20000U /**< 128 KB sector */

/**
 * @brief User data storage area (using last sector of Bank 1)
 * @note  Sector 11 (128KB) is typically safe for user data
 */
#define FLASH_USER_START_ADDRESS 0x080E0000U /**< Sector 11 start */
#define FLASH_USER_END_ADDRESS 0x080FFFFFU   /**< Sector 11 end */
#define FLASH_USER_SECTOR FLASH_SECTOR_11    /**< User data sector */
#define FLASH_USER_SIZE FLASH_SECTOR_SIZE_128KB

/**
 * @brief Flash operation timeout
 */
#define FLASH_TIMEOUT_VALUE 50000U /**< 50 seconds timeout */

/**
 * @brief Number of sectors
 */
#define FLASH_SECTOR_TOTAL 24U /**< Total sectors (both banks) */
#define FLASH_SECTOR_BANK1 12U /**< Sectors in Bank 1 */
#define FLASH_SECTOR_BANK2 12U /**< Sectors in Bank 2 */

/** @brief Returned by FLASH_GetSector when the address is outside Flash */
#define FLASH_SECTOR_NOT_FOUND 0xFFFFFFFFU

/** @brief Value read back from an erased Flash byte */
#define FLASH_ERASED_BYTE 0xFFU

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_TYPES_H__ */
