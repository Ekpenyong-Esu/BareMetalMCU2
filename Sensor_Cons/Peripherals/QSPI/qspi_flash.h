/**
  ******************************************************************************
  * @file    qspi_flash.h
  * @brief   Serial NOR flash command set and status register layout
  ******************************************************************************
  */

#ifndef QSPI_FLASH_H
#define QSPI_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Commands ------------------------------------------------------------------*/
#define QSPI_CMD_WRITE_ENABLE           0x06U
#define QSPI_CMD_WRITE_DISABLE          0x04U
#define QSPI_CMD_READ_STATUS_REG        0x05U
#define QSPI_CMD_WRITE_STATUS_REG       0x01U
#define QSPI_CMD_READ_DATA              0x03U
#define QSPI_CMD_FAST_READ              0x0BU
#define QSPI_CMD_QUAD_READ              0x6BU
#define QSPI_CMD_PAGE_PROGRAM           0x02U
#define QSPI_CMD_QUAD_PAGE_PROGRAM      0x32U
#define QSPI_CMD_SECTOR_ERASE           0x20U   /**< 4 KB */
#define QSPI_CMD_BLOCK_ERASE_32K        0x52U
#define QSPI_CMD_BLOCK_ERASE_64K        0xD8U
#define QSPI_CMD_CHIP_ERASE             0xC7U
#define QSPI_CMD_READ_ID                0x9FU   /**< JEDEC ID */
#define QSPI_CMD_READ_UNIQUE_ID         0x4BU
#define QSPI_CMD_DEEP_POWER_DOWN        0xB9U
#define QSPI_CMD_RELEASE_POWER_DOWN     0xABU
#define QSPI_CMD_RESET_ENABLE           0x66U
#define QSPI_CMD_RESET                  0x99U

/* Status register bits ------------------------------------------------------*/
#define QSPI_SR_BUSY                    0x01U
#define QSPI_SR_WEL                     0x02U   /**< Write enable latch */
#define QSPI_SR_BP0                     0x04U
#define QSPI_SR_BP1                     0x08U
#define QSPI_SR_BP2                     0x10U
#define QSPI_SR_TB                      0x20U
#define QSPI_SR_SEC                     0x40U
#define QSPI_SR_SRP0                    0x80U

/* Transfer sizes ------------------------------------------------------------*/
#define QSPI_JEDEC_ID_LENGTH            3U
#define QSPI_UNIQUE_ID_LENGTH           8U
#define QSPI_UNIQUE_ID_DUMMY_BYTES      4U
#define QSPI_STATUS_REG_SIZE            1U
#define QSPI_ADDRESS_BYTES              3U      /**< 24-bit addressing */

/* JEDEC manufacturer IDs ----------------------------------------------------*/
#define QSPI_MANUFACTURER_MICRON        0x20U
#define QSPI_MANUFACTURER_WINBOND       0xEFU
#define QSPI_MANUFACTURER_MACRONIX      0xC2U

#ifdef __cplusplus
}
#endif

#endif /* QSPI_FLASH_H */
