/**
  ******************************************************************************
  * @file    app_sdram_power.c
  * @brief   External SDRAM power state and re-initialization
  ******************************************************************************
  */

#include "app_sdram_power.h"
#include "log.h"
#include "fmc.h"

#define APP_SDRAM_STABILIZE_MS   10U

static bool sdram_is_active = true;

/* Static, not stack: the FMC driver keeps using this handle after PowerOn returns. */
static FMC_Driver_Handle_t s_fmcHandle;

/* Matches the on-board IS42S16400J wired to FMC bank 2. */
static const FMC_Driver_SDRAM_Config_t s_sdramConfig = {
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
    /* Read burst is off in the controller, so the device stays on single accesses. */
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

void APP_SDRAM_PowerOff(void)
{
    if (!sdram_is_active) {
        return;
    }

    log_debug("APP: Turning SDRAM off");

    /* No SDRAM handle is exported here; the MSP hooks ignore theirs and only
       touch the shared FMC clock and pins. */
    HAL_SDRAM_MspDeInit(NULL);

    sdram_is_active = false;
}

bool APP_SDRAM_PowerOn(void)
{
    if (sdram_is_active) {
        return true;
    }

    log_debug("APP: Turning SDRAM on");

    HAL_SDRAM_MspInit(NULL);
    HAL_Delay(APP_SDRAM_STABILIZE_MS);

    if (FMC_Driver_SDRAM_Init(&s_fmcHandle, &s_sdramConfig) != HAL_OK) {
        log_error("APP: SDRAM reinit failed");
        return false;
    }

    sdram_is_active = true;
    log_debug("APP: SDRAM reinitialized");

    return true;
}

bool APP_SDRAM_IsActive(void)
{
    return sdram_is_active;
}
