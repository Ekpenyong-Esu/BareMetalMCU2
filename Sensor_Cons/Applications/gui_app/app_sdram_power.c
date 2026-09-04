/**
 ******************************************************************************
 * @file    app_sdram_power.c
 * @brief   External SDRAM power state and re-initialization
 ******************************************************************************
 */

#include "app_sdram_power.h"
#include "gui_board.h"
#include "log.h"
#include "fmc.h"

#define APP_SDRAM_STABILIZE_MS 10U

static bool sdram_is_active = true;

void APP_SDRAM_PowerOff(void) {
    if (!sdram_is_active) {
        return;
    }

    log_debug("APP: Turning SDRAM off");

    /* No SDRAM handle is exported here; the MSP hooks ignore theirs and only
       touch the shared FMC clock and pins. */
    HAL_SDRAM_MspDeInit(NULL);

    sdram_is_active = false;
}

bool APP_SDRAM_PowerOn(void) {
    if (sdram_is_active) {
        return true;
    }

    log_debug("APP: Turning SDRAM on");

    HAL_SDRAM_MspInit(NULL);
    HAL_Delay(APP_SDRAM_STABILIZE_MS);

    /* The board owns the FMC handle and the device timings; re-run the same
       sequence it used at power-up. */
    if (!GUI_Board_SdramInit()) {
        log_error("APP: SDRAM reinit failed");
        return false;
    }

    sdram_is_active = true;
    log_debug("APP: SDRAM reinitialized");

    return true;
}

bool APP_SDRAM_IsActive(void) {
    return sdram_is_active;
}
