/**
 ******************************************************************************
 * @file    ssd1331.c
 * @brief   SSD1331 lifecycle and panel control
 ******************************************************************************
 */

#include "ssd1331_core.h"
#include "ssd1331_buffer.h"
#include "ssd1331_cmd.h"
#include "ssd1331_io.h"
#include "log.h"
#include <string.h>

#define SSD1331_MASTER_CURRENT_MAX 0x0FU

/* 96x64 bring-up, in the order the datasheet's application note gives. */
static const uint8_t SSD1331_InitSequence[] = {SSD1331_CMD_DISPLAY_OFF,
                                               SSD1331_CMD_SET_REMAP,
                                               SSD1331_REMAP_RGB565,
                                               SSD1331_CMD_SET_START_LINE,
                                               SSD1331_START_LINE_NONE,
                                               SSD1331_CMD_SET_DISPLAY_OFFSET,
                                               SSD1331_DISPLAY_OFFSET_NONE,
                                               SSD1331_CMD_NORMAL_DISPLAY,
                                               SSD1331_CMD_SET_MULTIPLEX,
                                               SSD1331_MULTIPLEX_64,
                                               SSD1331_CMD_SET_MASTER_CONFIG,
                                               SSD1331_MASTER_CONFIG_EXT_VCC,
                                               SSD1331_CMD_POWER_SAVE,
                                               SSD1331_POWER_SAVE_DISABLE,
                                               SSD1331_CMD_PHASE_PERIOD,
                                               SSD1331_PHASE_PERIOD_DEFAULT,
                                               SSD1331_CMD_SET_CLOCK_DIV,
                                               SSD1331_CLOCK_DIV_DEFAULT,
                                               SSD1331_CMD_PRECHARGE_A,
                                               SSD1331_PRECHARGE_SPEED_DEFAULT,
                                               SSD1331_CMD_PRECHARGE_B,
                                               SSD1331_PRECHARGE_SPEED_DEFAULT,
                                               SSD1331_CMD_PRECHARGE_C,
                                               SSD1331_PRECHARGE_SPEED_DEFAULT,
                                               SSD1331_CMD_SET_PRECHARGE_LEVEL,
                                               SSD1331_PRECHARGE_LEVEL_DEFAULT,
                                               SSD1331_CMD_SET_VCOMH,
                                               SSD1331_VCOMH_DEFAULT,
                                               SSD1331_CMD_MASTER_CURRENT,
                                               SSD1331_MASTER_CURRENT_DEFAULT,
                                               SSD1331_CMD_SET_CONTRAST_A,
                                               SSD1331_CONTRAST_A_DEFAULT,
                                               SSD1331_CMD_SET_CONTRAST_B,
                                               SSD1331_CONTRAST_B_DEFAULT,
                                               SSD1331_CMD_SET_CONTRAST_C,
                                               SSD1331_CONTRAST_C_DEFAULT,
                                               SSD1331_CMD_DEACTIVATE_SCROLL};

SSD1331_StatusTypeDef SSD1331_Init(SSD1331_Handle_t *hssd, const SSD1331_Config_t *config) {
    SSD1331_StatusTypeDef status = SSD1331_OK;

    if (hssd == NULL || config == NULL || config->bus == NULL) {
        return SSD1331_INVALID_PARAM;
    }

    log_info("SSD1331: initializing display");

    memset(hssd, 0, sizeof(SSD1331_Handle_t));
    hssd->config = *config;

    status = SSD1331_IO_Init(hssd);
    if (status != SSD1331_OK) {
        return status;
    }

    status = SSD1331_IO_WriteCommands(hssd, SSD1331_InitSequence, sizeof(SSD1331_InitSequence));
    if (status != SSD1331_OK) {
        return status;
    }

    /* The buffer and refresh calls both refuse an uninitialised handle. */
    hssd->initialized = true;

    /* Display RAM powers up with undefined content, so it has to be blanked
       before the panel is switched on, not just the RAM buffer. */
    (void)SSD1331_Clear(hssd);
    status = SSD1331_UpdateScreen(hssd);
    if (status != SSD1331_OK) {
        hssd->initialized = false;
        return status;
    }

    status = SSD1331_IO_WriteCommand(hssd, SSD1331_CMD_DISPLAY_ON);
    if (status != SSD1331_OK) {
        hssd->initialized = false;
        return status;
    }

    log_info("SSD1331: display initialized successfully");

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_DeInit(SSD1331_Handle_t *hssd) {
    SSD1331_StatusTypeDef status = SSD1331_OK;

    SSD1331_CHECK_HANDLE(hssd);

    status = SSD1331_IO_WriteCommand(hssd, SSD1331_CMD_DISPLAY_OFF);
    hssd->initialized = false;

    return status;
}

SSD1331_StatusTypeDef SSD1331_DisplayOn(SSD1331_Handle_t *hssd, bool enable) {
    SSD1331_CHECK_HANDLE(hssd);

    return SSD1331_IO_WriteCommand(hssd, enable ? SSD1331_CMD_DISPLAY_ON : SSD1331_CMD_DISPLAY_OFF);
}

SSD1331_StatusTypeDef SSD1331_SetContrast(SSD1331_Handle_t *hssd, uint8_t red, uint8_t green,
                                          uint8_t blue) {
    const uint8_t frame[] = {SSD1331_CMD_SET_CONTRAST_A, red, SSD1331_CMD_SET_CONTRAST_B, green,
                             SSD1331_CMD_SET_CONTRAST_C, blue};

    SSD1331_CHECK_HANDLE(hssd);

    return SSD1331_IO_WriteCommands(hssd, frame, sizeof(frame));
}

SSD1331_StatusTypeDef SSD1331_SetBrightness(SSD1331_Handle_t *hssd, uint8_t current) {
    uint8_t frame[2] = {SSD1331_CMD_MASTER_CURRENT, current};

    SSD1331_CHECK_HANDLE(hssd);

    if (current > SSD1331_MASTER_CURRENT_MAX) {
        return SSD1331_INVALID_PARAM;
    }

    return SSD1331_IO_WriteCommands(hssd, frame, sizeof(frame));
}

SSD1331_StatusTypeDef SSD1331_InvertDisplay(SSD1331_Handle_t *hssd, bool invert) {
    SSD1331_CHECK_HANDLE(hssd);

    return SSD1331_IO_WriteCommand(hssd, invert ? SSD1331_CMD_INVERT_DISPLAY
                                                : SSD1331_CMD_NORMAL_DISPLAY);
}
