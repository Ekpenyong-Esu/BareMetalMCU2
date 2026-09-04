/**
 * @file fmc_nor.c
 * @brief NOR Flash bring-up and access
 */

#include "fmc_nor.h"
#include "log.h"
#include <string.h>

/* NOR Flash command sequence (AMD/Spansion CFI standard), byte addresses for a
   16-bit device: the unlock cycles always target the base of the device, not
   the sector being erased. */
#define NOR_CMD_UNLOCK_ADDR1 0x0AAAU
#define NOR_CMD_UNLOCK_ADDR2 0x0554U
#define NOR_CMD_UNLOCK1 0xAAU
#define NOR_CMD_UNLOCK2 0x55U
#define NOR_CMD_ERASE 0x80U
#define NOR_CMD_ERASE_SECTOR 0x30U

/* A NOR cell reads back all-ones once the sector erase has finished */
#define NOR_ERASED_VALUE 0xFFFFU

uint32_t FMC_Driver_NOR_GetBase(const FMC_Driver_NOR_Config_t *config) {
    if (config == NULL) {
        return 0U;
    }

    switch (config->bank) {
        case FMC_NORSRAM_BANK1:
            return NOR_BANK1_BASE_ADDR;
        case FMC_NORSRAM_BANK2:
            return NOR_BANK1_BASE_ADDR + NOR_SUBBANK_SIZE;
        case FMC_NORSRAM_BANK3:
            return NOR_BANK1_BASE_ADDR + 2U * NOR_SUBBANK_SIZE;
        case FMC_NORSRAM_BANK4:
            return NOR_BANK1_BASE_ADDR + 3U * NOR_SUBBANK_SIZE;
        default:
            return 0U;
    }
}

HAL_StatusTypeDef FMC_Driver_NOR_Init(FMC_Driver_Handle_t *handle,
                                      const FMC_Driver_NOR_Config_t *config) {
    FMC_NORSRAM_TimingTypeDef timing = {0};
    uint32_t base = 0;

    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }

    base = FMC_Driver_NOR_GetBase(config);
    if (base == 0U) {
        log_error("FMC: invalid NOR bank");
        return HAL_ERROR;
    }

    /* HAL_SRAM_Init dereferences ExtTiming as soon as extended mode is on. */
    if (config->extendedMode == FMC_EXTENDED_MODE_ENABLE) {
        log_error(
            "FMC: NOR extended mode needs a second timing set, which this driver does not take");
        return HAL_ERROR;
    }

    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    handle->hsram.Instance = FMC_NORSRAM_DEVICE;
    handle->hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    handle->hsram.Init.NSBank = config->bank;
    handle->hsram.Init.DataAddressMux = config->dataAddressMux;
    handle->hsram.Init.MemoryType = config->memoryType;
    handle->hsram.Init.MemoryDataWidth = config->memoryDataWidth;
    handle->hsram.Init.BurstAccessMode = config->burstAccessMode;
    handle->hsram.Init.WaitSignalPolarity = config->waitSignalPolarity;
    handle->hsram.Init.WaitSignalActive = config->waitSignalActive;
    handle->hsram.Init.WriteOperation = config->writeOperation;
    handle->hsram.Init.WaitSignal = config->waitSignal;
    handle->hsram.Init.ExtendedMode = config->extendedMode;
    handle->hsram.Init.AsynchronousWait = config->asynchronousWait;
    handle->hsram.Init.WriteBurst = config->writeBurst;
    handle->hsram.Init.ContinuousClock = config->continuousClock;
    handle->hsram.Init.WriteFifo = config->writeFifo;
    handle->hsram.Init.PageSize = config->pageSize;

    timing.AddressSetupTime = config->addressSetupTime;
    timing.AddressHoldTime = config->addressHoldTime;
    timing.DataSetupTime = config->dataSetupTime;
    timing.BusTurnAroundDuration = config->busTurnAroundDuration;
    timing.CLKDivision = config->clkDivision;
    timing.DataLatency = config->dataLatency;
    timing.AccessMode = config->accessMode;

    if (HAL_SRAM_Init(&handle->hsram, &timing, NULL) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    handle->memoryType = FMC_DRIVER_MEMORY_NOR;
    handle->baseAddress = base;
    handle->deviceSize = NOR_SUBBANK_SIZE;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Common guard for the 16-bit wide NOR transfers
 */
static HAL_StatusTypeDef FMC_Driver_NOR_CheckTransfer(FMC_Driver_Handle_t *handle, uint32_t address,
                                                      const void *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized ||
        handle->memoryType != FMC_DRIVER_MEMORY_NOR) {
        return HAL_ERROR;
    }

    /* The device is 16 bits wide, and both pointers are dereferenced as
       16-bit words, so an odd count or an odd buffer would fault or lose a byte. */
    if ((size % 2U) != 0U || (address % 2U) != 0U || (((uintptr_t)data) % 2U) != 0U) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    if (address < handle->baseAddress ||
        size > handle->deviceSize - (address - handle->baseAddress)) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef FMC_Driver_NOR_Write(FMC_Driver_Handle_t *handle, uint32_t address,
                                       const uint8_t *data, uint32_t size) {
    HAL_StatusTypeDef status = FMC_Driver_NOR_CheckTransfer(handle, address, data, size);

    if (status != HAL_OK) {
        return status;
    }

    return HAL_SRAM_Write_16b(&handle->hsram, (uint32_t *)address, (uint16_t *)(uintptr_t)data,
                              size / 2U);
}

HAL_StatusTypeDef FMC_Driver_NOR_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data,
                                      uint32_t size) {
    HAL_StatusTypeDef status = FMC_Driver_NOR_CheckTransfer(handle, address, data, size);

    if (status != HAL_OK) {
        return status;
    }

    return HAL_SRAM_Read_16b(&handle->hsram, (uint32_t *)address, (uint16_t *)data, size / 2U);
}

HAL_StatusTypeDef FMC_Driver_NOR_EraseSector(FMC_Driver_Handle_t *handle, uint32_t address) {
    volatile uint16_t *pAddr = NULL;
    uint32_t deviceBase = 0;
    uint32_t startTick = 0;

    if (handle == NULL || !handle->initialized || handle->memoryType != FMC_DRIVER_MEMORY_NOR) {
        return HAL_ERROR;
    }

    if ((address % 2U) != 0U || address < handle->baseAddress ||
        (address - handle->baseAddress) >= handle->deviceSize) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    pAddr = (volatile uint16_t *)address;
    deviceBase = handle->baseAddress;

    *(volatile uint16_t *)(deviceBase + NOR_CMD_UNLOCK_ADDR1) = NOR_CMD_UNLOCK1;
    *(volatile uint16_t *)(deviceBase + NOR_CMD_UNLOCK_ADDR2) = NOR_CMD_UNLOCK2;
    *(volatile uint16_t *)(deviceBase + NOR_CMD_UNLOCK_ADDR1) = NOR_CMD_ERASE;
    *(volatile uint16_t *)(deviceBase + NOR_CMD_UNLOCK_ADDR1) = NOR_CMD_UNLOCK1;
    *(volatile uint16_t *)(deviceBase + NOR_CMD_UNLOCK_ADDR2) = NOR_CMD_UNLOCK2;
    *pAddr = NOR_CMD_ERASE_SECTOR;

    startTick = HAL_GetTick();
    while (*pAddr != NOR_ERASED_VALUE) {
        if ((HAL_GetTick() - startTick) > FMC_NOR_TIMEOUT) {
            handle->errorCode = FMC_DRIVER_ERROR_OPERATION;
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}
