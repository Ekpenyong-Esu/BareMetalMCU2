/**
 * @file qspi_types.h
 * @brief Types for external flash memory (QSPI)
 * @details This file is for talking to external flash memory. Flash is extra
 *          storage outside the chip, like a tiny SD card. QSPI is a fast way
 *          to read and write that flash. On this board there is no real QSPI
 *          hardware, so we use normal SPI with a chip select pin.
 *
 * How it works (in simple words):
 *  - The application opens an SPI_Bus_t and says which pin is chip select.
 *  - The driver registers the flash as a device on that bus.
 *  - Send a command to the flash chip over SPI.
 *  - Read or write data in pages and sectors.
 *  - Erase before you write again.
 */

#ifndef QSPI_TYPES_H
#define QSPI_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spi_core.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Geometry ------------------------------------------------------------------*/
#define QSPI_PAGE_SIZE 256U
#define QSPI_SECTOR_SIZE 4096U
#define QSPI_BLOCK_SIZE 65536U
#define QSPI_FLASH_SIZE_BYTES (16U * 1024U * 1024U)
#define QSPI_DEVICE_NAME_MAX_LENGTH 32U

/* Timeouts (ms) -------------------------------------------------------------*/
#define QSPI_TIMEOUT_DEFAULT 5000U
#define QSPI_WRITE_ENABLE_TIMEOUT 100U
#define QSPI_ERASE_TIMEOUT 10000U
#define QSPI_CHIP_ERASE_TIMEOUT 60000U

/* Transport -----------------------------------------------------------------*/
#define QSPI_DEFAULT_BAUDRATE_PRESCALER SPI_BAUDRATEPRESCALER_4
#define QSPI_HAL_MAX_TRANSFER 0xFFFFU /**< SPI_Transmit/Receive count is a uint16_t */

/* Status --------------------------------------------------------------------*/
typedef enum {
    QSPI_OK = 0x00,
    QSPI_ERROR = 0x01,
    QSPI_BUSY = 0x02,
    QSPI_TIMEOUT = 0x03,
    QSPI_INVALID_PARAM = 0x04,
    QSPI_NOT_SUPPORTED = 0x05,
    QSPI_WRITE_PROTECTED = 0x06,
    QSPI_ERASE_ERROR = 0x07,
    QSPI_PROGRAM_ERROR = 0x08
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
 * @brief Where the flash is wired and how fast to clock it.
 * @note  The bus is opened by the application and may be shared with other
 *        chips; only the chip-select line belongs to this driver.
 */
typedef struct {
    SPI_Bus_t *bus;       /**< Bus the flash hangs off, already opened */
    GPIO_TypeDef *csPort; /**< Chip select, driven in software */
    uint16_t csPin;
    uint32_t BaudRatePrescaler; /**< SPI_BAUDRATEPRESCALER_x dividing the bus clock */
} QSPI_ConfigTypeDef;

typedef struct {
    SPI_Device_t device; /**< Registration on the shared bus */
    QSPI_ConfigTypeDef Config;
    QSPI_MemoryInfoTypeDef MemInfo;
    uint32_t Timeout;
    bool IsInitialized;
} QSPI_HandleStructTypeDef;

/**
 * @brief Guard shared by every public entry point.
 */
static inline QSPI_StatusTypeDef QSPI_CheckReady(const QSPI_HandleStructTypeDef *hqspi) {
    if (hqspi == NULL || !hqspi->IsInitialized || !SPI_DeviceIsReady(&hqspi->device)) {
        return QSPI_INVALID_PARAM;
    }
    return QSPI_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* QSPI_TYPES_H */
