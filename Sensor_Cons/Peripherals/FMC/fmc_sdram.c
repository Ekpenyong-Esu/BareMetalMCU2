/**
 * @file fmc_sdram.c
 * @brief SDRAM bring-up and memory-mapped access
 */

#include "fmc_sdram.h"
#include "log.h"
#include <string.h>

/* JEDEC asks for a 100 us pause between clock enable and the first command;
   HAL_Delay's resolution is a millisecond, so one tick is the shortest wait. */
#define SDRAM_POWERUP_DELAY_MS  1U

/**
 * @brief Number of rows to be refreshed within SDRAM_REFRESH_PERIOD_MS
 */
static uint32_t FMC_Driver_SDRAM_RowCount(uint32_t rowBits)
{
    switch (rowBits) {
        case FMC_SDRAM_ROW_BITS_NUM_11: return 2048U;
        case FMC_SDRAM_ROW_BITS_NUM_12: return 4096U;
        case FMC_SDRAM_ROW_BITS_NUM_13: return 8192U;
        default:                        return 0U;
    }
}

static uint32_t FMC_Driver_SDRAM_ColumnCount(uint32_t columnBits)
{
    switch (columnBits) {
        case FMC_SDRAM_COLUMN_BITS_NUM_8:  return 256U;
        case FMC_SDRAM_COLUMN_BITS_NUM_9:  return 512U;
        case FMC_SDRAM_COLUMN_BITS_NUM_10: return 1024U;
        case FMC_SDRAM_COLUMN_BITS_NUM_11: return 2048U;
        default:                           return 0U;
    }
}

static uint32_t FMC_Driver_SDRAM_BusBytes(uint32_t dataWidth)
{
    switch (dataWidth) {
        case FMC_SDRAM_MEM_BUS_WIDTH_8:  return 1U;
        case FMC_SDRAM_MEM_BUS_WIDTH_16: return 2U;
        case FMC_SDRAM_MEM_BUS_WIDTH_32: return 4U;
        default:                         return 0U;
    }
}

/**
 * @brief SDRAM clock in Hz for a configured HCLK divider
 */
static uint32_t FMC_Driver_SDRAM_ClockHz(uint32_t clockPeriod)
{
    switch (clockPeriod) {
        case FMC_SDRAM_CLOCK_PERIOD_2: return HAL_RCC_GetHCLKFreq() / 2U;
        case FMC_SDRAM_CLOCK_PERIOD_3: return HAL_RCC_GetHCLKFreq() / 3U;
        default:                       return 0U;
    }
}

uint32_t FMC_Driver_SDRAM_GetBase(const FMC_Driver_SDRAM_Config_t *config)
{
    if (config == NULL) {
        return 0U;
    }

    return (config->bank == FMC_SDRAM_BANK1) ? SDRAM_BANK1_BASE_ADDR : SDRAM_BANK2_BASE_ADDR;
}

uint32_t FMC_Driver_SDRAM_GetSize(const FMC_Driver_SDRAM_Config_t *config)
{
    uint32_t rows;
    uint32_t columns;
    uint32_t busBytes;
    uint32_t banks;

    if (config == NULL) {
        return 0U;
    }

    rows = FMC_Driver_SDRAM_RowCount(config->rowBits);
    columns = FMC_Driver_SDRAM_ColumnCount(config->columnBits);
    busBytes = FMC_Driver_SDRAM_BusBytes(config->dataWidth);
    banks = (config->internalBanks == FMC_SDRAM_INTERN_BANKS_NUM_4) ? 4U : 2U;

    if (rows == 0U || columns == 0U || busBytes == 0U) {
        return 0U;
    }

    return rows * columns * banks * busBytes;
}

uint32_t FMC_Driver_SDRAM_GetRefreshCount(const FMC_Driver_SDRAM_Config_t *config)
{
    uint32_t sdclk;
    uint32_t rows;
    uint32_t count;

    if (config == NULL) {
        return 0U;
    }

    sdclk = FMC_Driver_SDRAM_ClockHz(config->clockPeriod);
    rows = FMC_Driver_SDRAM_RowCount(config->rowBits);
    if (sdclk == 0U || rows == 0U) {
        return 0U;
    }

    /* COUNT = (refresh window x SDCLK) / rows - margin, in SDRAM clock cycles. */
    count = ((sdclk / 1000U) * SDRAM_REFRESH_PERIOD_MS) / rows;
    if (count <= SDRAM_REFRESH_MARGIN) {
        return 0U;
    }
    count -= SDRAM_REFRESH_MARGIN;

    if (count < SDRAM_REFRESH_COUNT_MIN || count > SDRAM_REFRESH_COUNT_MAX) {
        return 0U;
    }

    return count;
}

FMC_Driver_SDRAM_Config_t FMC_Driver_SDRAM_GetDefaultConfig(void)
{
    FMC_Driver_SDRAM_Config_t config = {
        .bank = FMC_SDRAM_BANK2,
        .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
        .rowBits = FMC_SDRAM_ROW_BITS_NUM_12,
        .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
        .internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4,
        .casLatency = FMC_SDRAM_CAS_LATENCY_3,
        .clockPeriod = FMC_SDRAM_CLOCK_PERIOD_3,
        .readBurst = FMC_SDRAM_RBURST_DISABLE,
        .readPipeDelay = FMC_SDRAM_RPIPE_DELAY_1,
        .writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
        .burstLength = SDRAM_MODEREG_BURST_LENGTH_1,
        .writeBurstMode = SDRAM_MODEREG_WRITEBURST_MODE_SINGLE,
        .loadToActiveDelay = 2,
        .exitSelfRefreshDelay = 7,
        .selfRefreshTime = 4,
        .rowCycleDelay = 7,
        .writeRecoveryTime = 2,
        .rpDelay = 2,
        .rcdDelay = 2
    };

    return config;
}

/**
 * @brief Issue one SDRAM command and record the failure in the handle
 */
static HAL_StatusTypeDef FMC_Driver_SDRAM_Command(FMC_Driver_Handle_t *handle,
                                                  uint32_t mode, uint32_t target,
                                                  uint32_t autoRefresh, uint32_t modeRegister)
{
    FMC_SDRAM_CommandTypeDef command = {0};

    command.CommandMode = mode;
    command.CommandTarget = target;
    command.AutoRefreshNumber = autoRefresh;
    command.ModeRegisterDefinition = modeRegister;

    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef FMC_Driver_SDRAM_Init(FMC_Driver_Handle_t *handle,
                                        const FMC_Driver_SDRAM_Config_t *config)
{
    FMC_SDRAM_TimingTypeDef timing = {0};
    uint32_t commandTarget;
    uint32_t refreshCount;
    uint32_t deviceSize;
    uint32_t modeRegister;

    log_debug("FMC: Initializing SDRAM");

    if (handle == NULL || config == NULL) {
        log_error("FMC: Invalid SDRAM init parameters");
        return HAL_ERROR;
    }

    deviceSize = FMC_Driver_SDRAM_GetSize(config);
    refreshCount = FMC_Driver_SDRAM_GetRefreshCount(config);
    if (deviceSize == 0U || refreshCount == 0U) {
        log_error("FMC: SDRAM geometry or clock period cannot be honoured");
        return HAL_ERROR;
    }

    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

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

    timing.LoadToActiveDelay = config->loadToActiveDelay;
    timing.ExitSelfRefreshDelay = config->exitSelfRefreshDelay;
    timing.SelfRefreshTime = config->selfRefreshTime;
    timing.RowCycleDelay = config->rowCycleDelay;
    timing.WriteRecoveryTime = config->writeRecoveryTime;
    timing.RPDelay = config->rpDelay;
    timing.RCDDelay = config->rcdDelay;

    if (HAL_SDRAM_Init(&handle->hsdram, &timing) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    commandTarget = (config->bank == FMC_SDRAM_BANK1) ? FMC_SDRAM_CMD_TARGET_BANK1
                                                      : FMC_SDRAM_CMD_TARGET_BANK2;

    if (FMC_Driver_SDRAM_Command(handle, FMC_SDRAM_CMD_CLK_ENABLE,
                                 commandTarget, 1, 0) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_Delay(SDRAM_POWERUP_DELAY_MS);

    if (FMC_Driver_SDRAM_Command(handle, FMC_SDRAM_CMD_PALL,
                                 commandTarget, 1, 0) != HAL_OK) {
        return HAL_ERROR;
    }

    if (FMC_Driver_SDRAM_Command(handle, FMC_SDRAM_CMD_AUTOREFRESH_MODE,
                                 commandTarget, 4, 0) != HAL_OK) {
        return HAL_ERROR;
    }

    modeRegister = (uint32_t)config->burstLength |
                   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                   ((config->casLatency == FMC_SDRAM_CAS_LATENCY_2)
                        ? SDRAM_MODEREG_CAS_LATENCY_2 : SDRAM_MODEREG_CAS_LATENCY_3) |
                   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                   (uint32_t)config->writeBurstMode;

    if (FMC_Driver_SDRAM_Command(handle, FMC_SDRAM_CMD_LOAD_MODE,
                                 commandTarget, 1, modeRegister) != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_SDRAM_ProgramRefreshRate(&handle->hsdram, refreshCount) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    handle->memoryType = FMC_DRIVER_MEMORY_SDRAM;
    handle->sdramConfig = *config;
    handle->baseAddress = FMC_Driver_SDRAM_GetBase(config);
    handle->deviceSize = deviceSize;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    log_debug("FMC: SDRAM initialized, %lu KB at 0x%08lX, refresh count %lu",
              (unsigned long)(deviceSize / 1024U), (unsigned long)handle->baseAddress,
              (unsigned long)refreshCount);

    return HAL_OK;
}

/**
 * @brief Reject a transfer that would leave the mapped SDRAM window
 */
static bool FMC_Driver_SDRAM_InRange(const FMC_Driver_Handle_t *handle,
                                     uint32_t address, uint32_t size)
{
    if (address < handle->baseAddress) {
        return false;
    }

    /* Compared against the remaining space so address + size cannot wrap. */
    return (size <= handle->deviceSize - (address - handle->baseAddress));
}

HAL_StatusTypeDef FMC_Driver_SDRAM_Write(FMC_Driver_Handle_t *handle, uint32_t address,
                                         const uint8_t *data, uint32_t size)
{
    if (handle == NULL || data == NULL || !handle->initialized ||
        handle->memoryType != FMC_DRIVER_MEMORY_SDRAM) {
        return HAL_ERROR;
    }

    if (!FMC_Driver_SDRAM_InRange(handle, address, size)) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    /* SDRAM is memory-mapped, so the copy is the transfer. */
    memcpy((void *)address, data, size);

    return HAL_OK;
}

HAL_StatusTypeDef FMC_Driver_SDRAM_Read(FMC_Driver_Handle_t *handle, uint32_t address,
                                        uint8_t *data, uint32_t size)
{
    if (handle == NULL || data == NULL || !handle->initialized ||
        handle->memoryType != FMC_DRIVER_MEMORY_SDRAM) {
        return HAL_ERROR;
    }

    if (!FMC_Driver_SDRAM_InRange(handle, address, size)) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    memcpy(data, (const void *)address, size);

    return HAL_OK;
}

bool FMC_Driver_SDRAM_Test(FMC_Driver_Handle_t *handle, uint32_t startAddr, uint32_t size)
{
    uint16_t *pMem;
    uint32_t numWords;

    if (handle == NULL || !handle->initialized ||
        handle->memoryType != FMC_DRIVER_MEMORY_SDRAM) {
        return false;
    }

    if ((startAddr % 2U) != 0U || !FMC_Driver_SDRAM_InRange(handle, startAddr, size)) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return false;
    }

    pMem = (uint16_t *)startAddr;
    numWords = size / 2U;

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
