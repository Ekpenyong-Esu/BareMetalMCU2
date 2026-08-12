/**
 * @file    eeprom_io.c
 * @brief   EEPROM transport layer over the shared I2C bus
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_io.h"
#include "i2c.h"

/* Private defines -----------------------------------------------------------*/
/** A 1-byte memory address can only span 256 bytes; anything beyond that is
 *  selected through the low bits of the I2C device address (M24C04/08/16). */
#define EEPROM_BLOCK_SIZE       256U

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Number of memory-block bits that live in the I2C device address
 */
static uint8_t EEPROM_IO_BlockBits(const EEPROM_HandleTypeDef* handle)
{
    uint8_t bits = 0;
    uint32_t blocks;

    if (handle->config.addressSize != 1U)
    {
        return 0;
    }

    for (blocks = handle->config.totalSize / EEPROM_BLOCK_SIZE; blocks > 1U; blocks >>= 1)
    {
        bits++;
    }

    return bits;
}

/**
 * @brief 8-bit I2C device address for the block containing @p address
 */
static uint16_t EEPROM_IO_DeviceAddress(const EEPROM_HandleTypeDef* handle,
                                        uint16_t address,
                                        uint8_t blockBits)
{
    const uint8_t blockMask = (uint8_t)((1U << blockBits) - 1U);
    const uint8_t block = (uint8_t)((address / EEPROM_BLOCK_SIZE) & blockMask);

    return (uint16_t)((handle->activeAddress | block) << 1);
}

/**
 * @brief Bytes left in the current block, i.e. the largest safe transfer
 */
static uint16_t EEPROM_IO_ChunkLength(uint16_t address, uint16_t length, uint8_t blockBits)
{
    uint16_t remaining;

    if (blockBits == 0U)
    {
        return length;
    }

    remaining = (uint16_t)(EEPROM_BLOCK_SIZE - (address % EEPROM_BLOCK_SIZE));

    return (length < remaining) ? length : remaining;
}

/**
 * @brief Address byte(s) sent on the bus; block bits have moved to the device address
 */
static uint16_t EEPROM_IO_MemAddress(uint16_t address, uint8_t blockBits)
{
    return (blockBits == 0U) ? address : (uint16_t)(address % EEPROM_BLOCK_SIZE);
}

/* Public functions ----------------------------------------------------------*/

void EEPROM_IO_Init(void)
{
    if (!I2C_IsReady())
    {
        I2C_Init();
    }
}

EEPROM_StatusTypeDef EEPROM_IO_Write(const EEPROM_HandleTypeDef* handle,
                                     uint16_t address,
                                     const uint8_t* data,
                                     uint16_t length)
{
    const uint8_t blockBits = EEPROM_IO_BlockBits(handle);
    uint16_t done = 0;

    while (done < length)
    {
        const uint16_t offset = (uint16_t)(address + done);
        const uint16_t chunk = EEPROM_IO_ChunkLength(offset, (uint16_t)(length - done), blockBits);
        uint16_t devAddr = EEPROM_IO_DeviceAddress(handle, offset, blockBits);
        uint16_t memAddr = EEPROM_IO_MemAddress(offset, blockBits);

        if (I2C_Mem_Write(devAddr,
                          memAddr,
                          handle->config.addressSize,
                          (uint8_t*)(uintptr_t)(data + done),
                          chunk,
                          EEPROM_TIMEOUT_DEFAULT) != I2C_OK)
        {
            return EEPROM_ERROR;
        }

        done = (uint16_t)(done + chunk);
    }

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_IO_Read(const EEPROM_HandleTypeDef* handle,
                                    uint16_t address,
                                    uint8_t* data,
                                    uint16_t length)
{
    const uint8_t blockBits = EEPROM_IO_BlockBits(handle);
    uint16_t done = 0;

    while (done < length)
    {
        const uint16_t offset = (uint16_t)(address + done);
        const uint16_t chunk = EEPROM_IO_ChunkLength(offset, (uint16_t)(length - done), blockBits);
        uint16_t devAddr = EEPROM_IO_DeviceAddress(handle, offset, blockBits);
        uint16_t memAddr = EEPROM_IO_MemAddress(offset, blockBits);
        uint8_t *readPos = (uint8_t *)(uintptr_t)(data + done);

        if (I2C_Mem_Read(devAddr,
                         memAddr,
                         handle->config.addressSize,
                         readPos,
                         chunk,
                         EEPROM_TIMEOUT_DEFAULT) != I2C_OK)
        {
            return EEPROM_ERROR;
        }

        done = (uint16_t)(done + chunk);
    }

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_IO_IsDeviceReady(const EEPROM_HandleTypeDef* handle)
{
    const uint16_t devAddr = (uint16_t)(handle->activeAddress << 1);

    if (I2C_IsDeviceReady(devAddr, EEPROM_MAX_TRIALS, EEPROM_TIMEOUT_DEFAULT) != I2C_OK)
    {
        return EEPROM_TIMEOUT;
    }

    return EEPROM_OK;
}
