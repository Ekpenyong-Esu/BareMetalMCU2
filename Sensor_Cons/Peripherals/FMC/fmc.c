/**
 * @file fmc.c
 * @brief Flexible Memory Controller (FMC) Driver Implementation
 * @author Generated for STM32F429
 * @date 2025
 *
 * This FMC driver provides implementation for external memory operations
 * on STM32F4 series, supporting SDRAM, NOR Flash, and NAND Flash memories.
 * Built on top of STM32 HAL library following best practices.
 */

#include "fmc.h"
#include <string.h>
#include "log.h"

/* Private defines -----------------------------------------------------------*/

/* NOR Flash erase command sequence (AMD/Spansion CFI standard) */
#define NOR_CMD_UNLOCK_ADDR1    0x0AAA
#define NOR_CMD_UNLOCK_ADDR2    0x0554
#define NOR_CMD_UNLOCK1         0xAA
#define NOR_CMD_UNLOCK2         0x55
#define NOR_CMD_ERASE          0x80
#define NOR_CMD_ERASE_SECTOR   0x30

/* A NOR cell reads back all-ones once the sector erase has finished */
#define NOR_ERASED_VALUE       0xFFFFU
#define NOR_SECTOR_BASE_MASK   0xFFFF0000U

/* Private function prototypes -----------------------------------------------*/

/*=============================================================================
 * SDRAM Functions
 *===========================================================================*/

/**
 * @brief Initialize FMC driver with SDRAM configuration
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Init(FMC_Driver_Handle_t *handle, FMC_Driver_SDRAM_Config_t *config) {
    log_debug("FMC: Initializing SDRAM");
    if (handle == NULL || config == NULL) {
        log_error("FMC: Invalid SDRAM init parameters");
        return HAL_ERROR;
    }

    /* Clear the handle to avoid using uninitialized fields */
    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    /* Configure SDRAM device instance */
    handle->hsdram.Instance = FMC_SDRAM_DEVICE;
    handle->hsdram.Init.SDBank = config->bank;
    handle->hsdram.Init.ColumnBitsNumber = config->columnBits;
    handle->hsdram.Init.RowBitsNumber = config->rowBits;
    handle->hsdram.Init.MemoryDataWidth = config->dataWidth;
    handle->hsdram.Init.InternalBankNumber = config->internalBanks;
    handle->hsdram.Init.CASLatency = config->casLatency;
    handle->hsdram.Init.WriteProtection = config->writeProtection;
    handle->hsdram.Init.SDClockPeriod = config->clockPeriod;
    handle->hsdram.Init.ReadBurst = config->readBurst;
    handle->hsdram.Init.ReadPipeDelay = config->readPipeDelay;

    /* SDRAM timing configuration */
    FMC_SDRAM_TimingTypeDef timing;
    timing.LoadToActiveDelay = config->loadToActiveDelay;
    timing.ExitSelfRefreshDelay = config->exitSelfRefreshDelay;
    timing.SelfRefreshTime = config->selfRefreshTime;
    timing.RowCycleDelay = config->rowCycleDelay;
    timing.WriteRecoveryTime = config->writeRecoveryTime;
    timing.RPDelay = config->rpDelay;
    timing.RCDDelay = config->rcdDelay;

    /* Initialize SDRAM controller */
    if (HAL_SDRAM_Init(&handle->hsdram, &timing) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* SDRAM initialization sequence */
    FMC_SDRAM_CommandTypeDef command;
    uint32_t commandTarget = 0;

    /* Map Bank to Command Target */
    if (config->bank == FMC_SDRAM_BANK1) {
        commandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    } else {
        commandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    }

    /* Step 1: Configure clock enable command */
    command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Delay (100µs minimum) */
    HAL_Delay(100);

    /* Step 2: Configure PALL (precharge all) command */
    command.CommandMode = FMC_SDRAM_CMD_PALL;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Step 3: Configure auto-refresh command */
    command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 4;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Step 4: Program mode register */
    volatile uint32_t modeReg = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
                       SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                       (config->casLatency == FMC_SDRAM_CAS_LATENCY_2 ? SDRAM_MODEREG_CAS_LATENCY_2 : SDRAM_MODEREG_CAS_LATENCY_3) |
                       SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                       SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = modeReg;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Step 5: Set refresh rate (64ms / 4096 rows = 15.625µs per row)
     * Refresh rate = (15.625µs × SDRAM_CLK) - 20
     * For 84MHz SDRAM_CLK: (15.625µs × 84MHz) - 20 = 1292 */
    if (HAL_SDRAM_ProgramRefreshRate(&handle->hsdram, SDRAM_REFRESH_COUNT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Update handle state */
    handle->memoryType = FMC_DRIVER_MEMORY_SDRAM;
    handle->sdramConfig = *config;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    log_debug("FMC: SDRAM initialized successfully");
    return HAL_OK;
}

/**
 * @brief Write data to SDRAM
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* SDRAM is memory-mapped, so the copy is the transfer. The former DMA path
       dereferenced hsdram.hdma, which nothing ever links, and passed size / 4,
       silently dropping any trailing bytes. */
    memcpy((void *)address, data, size);
    return HAL_OK;
}

/**
 * @brief Read data from SDRAM
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    memcpy(data, (const void *)address, size);
    return HAL_OK;
}

/**
 * @brief Test SDRAM memory
 */
bool FMC_Driver_SDRAM_Test(FMC_Driver_Handle_t *handle, uint32_t startAddr, uint32_t size) {
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    /* Pattern test using 16-bit accesses (SDRAM is 16-bit wide) */
    uint16_t *pMem = (uint16_t *)startAddr;
    uint32_t numWords = size / 2;

    /* Derive the pattern from the index. A single constant would still pass
       with a stuck or shorted address line, because every cell holds the
       same value. */
    for (uint32_t i = 0; i < numWords; i++) {
        pMem[i] = (uint16_t)(i ^ (i >> 16));
    }

    for (uint32_t i = 0; i < numWords; i++) {
        if (pMem[i] != (uint16_t)(i ^ (i >> 16))) {
            return false;
        }
    }

    /* Inverse pattern test */
    for (uint32_t i = 0; i < numWords; i++) {
        pMem[i] = (uint16_t)~(i ^ (i >> 16));
    }

    for (uint32_t i = 0; i < numWords; i++) {
        if (pMem[i] != (uint16_t)~(i ^ (i >> 16))) {
            return false;
        }
    }

    return true;
}

/*=============================================================================
 * NOR Flash Functions
 *===========================================================================*/

/**
 * @brief Initialize FMC driver with NOR Flash configuration
 */
HAL_StatusTypeDef FMC_Driver_NOR_Init(FMC_Driver_Handle_t *handle, FMC_Driver_NOR_Config_t *config) {
    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Clear the handle to avoid using uninitialized fields */
    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    /* Configure NOR Flash device */
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

    /* Timing configuration */
    FMC_NORSRAM_TimingTypeDef timing;
    timing.AddressSetupTime = config->addressSetupTime;
    timing.AddressHoldTime = config->addressHoldTime;
    timing.DataSetupTime = config->dataSetupTime;
    timing.BusTurnAroundDuration = config->busTurnAroundDuration;
    timing.CLKDivision = config->clkDivision;
    timing.DataLatency = config->dataLatency;
    timing.AccessMode = config->accessMode;

    /* Initialize NOR controller */
    if (HAL_SRAM_Init(&handle->hsram, &timing, NULL) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* Update handle state */
    handle->memoryType = FMC_DRIVER_MEMORY_NOR;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Write data to NOR Flash
 */
HAL_StatusTypeDef FMC_Driver_NOR_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* The device is 16 bits wide, so an odd count would silently lose its last byte */
    if ((size % 2U) != 0U) {
        return HAL_ERROR;
    }

    /* Write data using HAL SRAM functions */
    return HAL_SRAM_Write_16b(&handle->hsram, (uint32_t *)address, (uint16_t *)data, size / 2);
}

/**
 * @brief Read data from NOR Flash
 */
HAL_StatusTypeDef FMC_Driver_NOR_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    if ((size % 2U) != 0U) {
        return HAL_ERROR;
    }

    /* Read data using HAL SRAM functions */
    return HAL_SRAM_Read_16b(&handle->hsram, (uint32_t *)address, (uint16_t *)data, size / 2);
}

/**
 * @brief Erase NOR Flash sector
 */
HAL_StatusTypeDef FMC_Driver_NOR_EraseSector(FMC_Driver_Handle_t *handle, uint32_t address) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* NOR Flash erase requires specific command sequence */
    /* This is device-specific and should be customized based on NOR chip */
    volatile uint16_t *pAddr = (uint16_t *)address;
    uint32_t sectorBase = address & NOR_SECTOR_BASE_MASK;

    /* Standard AMD/Spansion erase sequence */
    *(volatile uint16_t *)(sectorBase | NOR_CMD_UNLOCK_ADDR1) = NOR_CMD_UNLOCK1;
    *(volatile uint16_t *)(sectorBase | NOR_CMD_UNLOCK_ADDR2) = NOR_CMD_UNLOCK2;
    *(volatile uint16_t *)(sectorBase | NOR_CMD_UNLOCK_ADDR1) = NOR_CMD_ERASE;
    *(volatile uint16_t *)(sectorBase | NOR_CMD_UNLOCK_ADDR1) = NOR_CMD_UNLOCK1;
    *(volatile uint16_t *)(sectorBase | NOR_CMD_UNLOCK_ADDR2) = NOR_CMD_UNLOCK2;
    *pAddr = NOR_CMD_ERASE_SECTOR;

    /* Wait for erase completion. The previous `while (... && timeout-- > 0)`
       wrapped timeout to UINT32_MAX on the final test, so the closing
       `timeout > 0` check reported HAL_OK for every timeout. */
    uint32_t startTick = HAL_GetTick();
    while (*pAddr != NOR_ERASED_VALUE) {
        if ((HAL_GetTick() - startTick) > FMC_NOR_TIMEOUT) {
            handle->errorCode = FMC_DRIVER_ERROR_OPERATION;
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

/*=============================================================================
 * NAND Flash Functions
 *===========================================================================*/

/**
 * @brief Initialize FMC driver with NAND Flash configuration
 */
HAL_StatusTypeDef FMC_Driver_NAND_Init(FMC_Driver_Handle_t *handle, FMC_Driver_NAND_Config_t *config) {
    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Clear the handle to avoid using uninitialized fields */
    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    /* Configure NAND Flash device */
    handle->hnand.Instance = FMC_NAND_DEVICE;
    handle->hnand.Init.NandBank = config->bank;
    handle->hnand.Init.Waitfeature = config->waitFeature;
    handle->hnand.Init.MemoryDataWidth = config->memoryDataWidth;
    handle->hnand.Init.EccComputation = config->eccComputation;
    handle->hnand.Init.ECCPageSize = config->eccPageSize;
    handle->hnand.Init.TCLRSetupTime = config->tadl;
    handle->hnand.Init.TARSetupTime = config->thold;

    /* Timing configuration */
    FMC_NAND_PCC_TimingTypeDef timing;
    timing.SetupTime = config->tset;
    timing.WaitSetupTime = config->twait;
    timing.HoldSetupTime = config->thold;
    timing.HiZSetupTime = config->tadl;

    /* Initialize NAND controller */
    if (HAL_NAND_Init(&handle->hnand, &timing, &timing) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* Update handle state */
    handle->memoryType = FMC_DRIVER_MEMORY_NAND;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Write data to NAND Flash
 */
HAL_StatusTypeDef FMC_Driver_NAND_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Convert address to NAND address structure */
    NAND_AddressTypeDef nandAddr;
    nandAddr.Page = address & 0xFF;
    nandAddr.Plane = 0;
    nandAddr.Block = (address >> 8) & 0xFF;

    /* Write page */
    (void)size; /* Unused in page-based operation */
    return HAL_NAND_Write_Page_8b(&handle->hnand, &nandAddr, (uint8_t *)data, 1);
}

/**
 * @brief Read data from NAND Flash
 */
HAL_StatusTypeDef FMC_Driver_NAND_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Convert address to NAND address structure */
    NAND_AddressTypeDef nandAddr;
    nandAddr.Page = address & 0xFF;
    nandAddr.Plane = 0;
    nandAddr.Block = (address >> 8) & 0xFF;

    /* Read page */
    (void)size; /* Unused in page-based operation */
    return HAL_NAND_Read_Page_8b(&handle->hnand, &nandAddr, data, 1);
}

/**
 * @brief Erase NAND Flash block
 */
HAL_StatusTypeDef FMC_Driver_NAND_EraseBlock(FMC_Driver_Handle_t *handle, uint32_t address) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Convert address to NAND address structure */
    NAND_AddressTypeDef nandAddr;
    nandAddr.Page = 0;
    nandAddr.Plane = 0;
    nandAddr.Block = (address >> 8) & 0xFF;

    /* Erase block */
    return HAL_NAND_Erase_Block(&handle->hnand, &nandAddr);
}

/*=============================================================================
 * Common Functions
 *===========================================================================*/

/**
 * @brief Deinitialize FMC driver
 */
HAL_StatusTypeDef FMC_Driver_DeInit(FMC_Driver_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_OK;

    /* Deinitialize based on memory type */
    if (handle->memoryType & FMC_DRIVER_MEMORY_SDRAM) {
        status = HAL_SDRAM_DeInit(&handle->hsdram);
    }
    if (handle->memoryType & FMC_DRIVER_MEMORY_NOR) {
        status = HAL_SRAM_DeInit(&handle->hsram);
    }
    if (handle->memoryType & FMC_DRIVER_MEMORY_NAND) {
        status = HAL_NAND_DeInit(&handle->hnand);
    }

    /* Clear handle */
    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    return status;
}

/**
 * @brief Get FMC driver error status
 */
uint32_t FMC_Driver_GetError(FMC_Driver_Handle_t *handle) {
    if (handle == NULL) {
        return FMC_DRIVER_ERROR_INIT;
    }
    return handle->errorCode;
}


