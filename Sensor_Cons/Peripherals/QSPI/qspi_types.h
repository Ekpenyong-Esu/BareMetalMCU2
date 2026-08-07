/**
  ******************************************************************************
  * @file    qspi_types.h
  * @brief   Shared vocabulary for the serial NOR flash driver
  * @note    The STM32F429 has no QUADSPI peripheral; this driver talks to the
  *          flash over plain SPI with a software chip select.
  ******************************************************************************
  */

#ifndef QSPI_TYPES_H
#define QSPI_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Geometry ------------------------------------------------------------------*/
#define QSPI_PAGE_SIZE                  256U
#define QSPI_SECTOR_SIZE                4096U
#define QSPI_BLOCK_SIZE                 65536U
#define QSPI_FLASH_SIZE_BYTES           (16U * 1024U * 1024U)
#define QSPI_DEVICE_NAME_MAX_LENGTH     32U

/* Timeouts (ms) -------------------------------------------------------------*/
#define QSPI_TIMEOUT_DEFAULT            5000U
#define QSPI_WRITE_ENABLE_TIMEOUT       100U
#define QSPI_ERASE_TIMEOUT              10000U
#define QSPI_CHIP_ERASE_TIMEOUT         60000U

/* Transport -----------------------------------------------------------------*/
#define QSPI_DEFAULT_BAUDRATE_PRESCALER SPI_BAUDRATEPRESCALER_4
#define QSPI_HAL_MAX_TRANSFER           0xFFFFU /**< HAL_SPI_* count is a uint16_t */

/* Status --------------------------------------------------------------------*/
typedef enum {
    QSPI_OK              = 0x00,
    QSPI_ERROR           = 0x01,
    QSPI_BUSY            = 0x02,
    QSPI_TIMEOUT         = 0x03,
    QSPI_INVALID_PARAM   = 0x04,
    QSPI_NOT_SUPPORTED   = 0x05,
    QSPI_WRITE_PROTECTED = 0x06,
    QSPI_ERASE_ERROR     = 0x07,
    QSPI_PROGRAM_ERROR   = 0x08
} QSPI_StatusTypeDef;

/* Device description --------------------------------------------------------*/
typedef struct {
    uint32_t FlashSize;
    uint32_t PageSize;
    uint32_t SectorSize;
    uint32_t BlockSize;
    uint8_t ManufacturerID;
    uint8_t DeviceID1;
    uint8_t DeviceID2;
    char DeviceName[QSPI_DEVICE_NAME_MAX_LENGTH];
} QSPI_MemoryInfoTypeDef;

/**
 * @brief Transport configuration.
 * @note  Bus speed is the only knob: the flash is driven over plain SPI.
 */
typedef struct {
    uint32_t BaudRatePrescaler;     /**< SPI_BAUDRATEPRESCALER_x dividing PCLK2 */
} QSPI_ConfigTypeDef;

typedef struct {
    SPI_HandleTypeDef *hspi;
    QSPI_ConfigTypeDef Config;
    QSPI_MemoryInfoTypeDef MemInfo;
    uint32_t Timeout;
    bool IsInitialized;
} QSPI_HandleStructTypeDef;

/**
 * @brief Guard shared by every public entry point.
 */
static inline QSPI_StatusTypeDef QSPI_CheckReady(const QSPI_HandleStructTypeDef *hqspi)
{
    if (hqspi == NULL || !hqspi->IsInitialized || hqspi->hspi == NULL) {
        return QSPI_INVALID_PARAM;
    }
    return QSPI_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* QSPI_TYPES_H */
