/**
 * @file    eeprom.c
 * @brief   EEPROM lifecycle, device geometry and readiness
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_core.h"
#include "eeprom_io.h"

/* Private constants ---------------------------------------------------------*/

/** Generic M24Cxx / AT24Cxx base address; A2..A0 are strapped or block bits */
#define EEPROM_I2C_ADDRESS_BASE     0x50U

/**
 * @brief Geometry of every supported device, indexed by EEPROM_TypeDef
 */
static const EEPROM_ConfigTypeDef eepromConfigs[] = {
    [EEPROM_TYPE_M24LR64] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_A01,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_A02,
        .totalSize = EEPROM_SIZE_8KB,
        .pageSize = EEPROM_PAGESIZE_4,
        .addressSize = 2
    },
    [EEPROM_TYPE_M24C01] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_128B,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1
    },
    [EEPROM_TYPE_M24C02] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_256B,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1
    },
    [EEPROM_TYPE_M24C04] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_512B,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1
    },
    [EEPROM_TYPE_M24C08] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_1KB,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1
    },
    [EEPROM_TYPE_M24C16] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_2KB,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1
    },
    [EEPROM_TYPE_M24C32] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_4KB,
        .pageSize = EEPROM_PAGESIZE_32,
        .addressSize = 2
    },
    [EEPROM_TYPE_M24C64] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_8KB,
        .pageSize = EEPROM_PAGESIZE_32,
        .addressSize = 2
    },
    [EEPROM_TYPE_M24C128] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_16KB,
        .pageSize = EEPROM_PAGESIZE_64,
        .addressSize = 2
    },
    [EEPROM_TYPE_M24C256] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_32KB,
        .pageSize = EEPROM_PAGESIZE_64,
        .addressSize = 2
    },
    [EEPROM_TYPE_M24C512] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_64KB,
        .pageSize = EEPROM_PAGESIZE_128,
        .addressSize = 2
    },
    [EEPROM_TYPE_AT24C256] = {
        .i2cAddress = EEPROM_I2C_ADDRESS_BASE,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_BASE,
        .totalSize = EEPROM_SIZE_32KB,
        .pageSize = EEPROM_PAGESIZE_64,
        .addressSize = 2
    }
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Probe the primary address, then the alternate one
 */
static EEPROM_StatusTypeDef EEPROM_Probe(EEPROM_HandleTypeDef* handle)
{
    EEPROM_IO_Init();

    handle->activeAddress = handle->config.i2cAddress;
    if (EEPROM_IO_IsDeviceReady(handle) == EEPROM_OK)
    {
        handle->initialized = true;
        return EEPROM_OK;
    }

    handle->activeAddress = handle->config.i2cAddressAlt;
    if (EEPROM_IO_IsDeviceReady(handle) == EEPROM_OK)
    {
        handle->initialized = true;
        return EEPROM_OK;
    }

    handle->initialized = false;

    return EEPROM_ERROR;
}

/* Public functions ----------------------------------------------------------*/

EEPROM_StatusTypeDef EEPROM_Init(EEPROM_HandleTypeDef* handle)
{
    return EEPROM_InitType(handle, EEPROM_TYPE_M24LR64);
}

EEPROM_StatusTypeDef EEPROM_InitType(EEPROM_HandleTypeDef* handle, EEPROM_TypeDef type)
{
    if (handle == NULL || type >= EEPROM_TYPE_CUSTOM)
    {
        return EEPROM_INVALID_PARAM;
    }

    handle->config = eepromConfigs[type];
    handle->type = type;

    return EEPROM_Probe(handle);
}

EEPROM_StatusTypeDef EEPROM_InitCustom(EEPROM_HandleTypeDef* handle,
                                       const EEPROM_ConfigTypeDef* config)
{
    if (handle == NULL || config == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    if (config->totalSize == 0 || config->pageSize == 0)
    {
        return EEPROM_INVALID_PARAM;
    }

    if (config->addressSize != 1 && config->addressSize != 2)
    {
        return EEPROM_INVALID_PARAM;
    }

    handle->config = *config;
    handle->type = EEPROM_TYPE_CUSTOM;

    return EEPROM_Probe(handle);
}

EEPROM_StatusTypeDef EEPROM_DeInit(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    handle->initialized = false;

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_CheckRange(const EEPROM_HandleTypeDef* handle,
                                       uint16_t address, uint16_t length)
{
    if (handle == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized)
    {
        return EEPROM_NOT_INITIALIZED;
    }

    if (((uint32_t)address + length) > handle->config.totalSize)
    {
        return EEPROM_INVALID_ADDRESS;
    }

    return EEPROM_OK;
}

EEPROM_StatusTypeDef EEPROM_WaitReady(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL)
    {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized)
    {
        return EEPROM_NOT_INITIALIZED;
    }

    return (EEPROM_IO_IsDeviceReady(handle) == EEPROM_OK) ? EEPROM_OK : EEPROM_TIMEOUT;
}

bool EEPROM_IsReady(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized)
    {
        return false;
    }

    return (EEPROM_IO_IsDeviceReady(handle) == EEPROM_OK);
}

uint32_t EEPROM_GetSize(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized)
    {
        return 0;
    }

    return handle->config.totalSize;
}

uint16_t EEPROM_GetPageSize(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized)
    {
        return 0;
    }

    return handle->config.pageSize;
}
