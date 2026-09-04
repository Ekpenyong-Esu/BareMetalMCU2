/**
 ******************************************************************************
 * @file    ssd1306.c
 * @brief   SSD1306 lifecycle and panel control
 ******************************************************************************
 */

#include "ssd1306_core.h"
#include "ssd1306_buffer.h"
#include "ssd1306_cmd.h"
#include "ssd1306_io.h"
#include "log.h"
#include <string.h>

/* 128x64 bring-up, in the order the datasheet's application note gives. */
static const uint8_t SSD1306_InitSequence[] = {SSD1306_CMD_DISPLAY_OFF,
                                               SSD1306_CMD_SET_DISPLAY_CLOCK_DIV,
                                               SSD1306_CLOCK_DIV_DEFAULT,
                                               SSD1306_CMD_SET_MULTIPLEX,
                                               SSD1306_MULTIPLEX_64,
                                               SSD1306_CMD_SET_DISPLAY_OFFSET,
                                               SSD1306_DISPLAY_OFFSET_NONE,
                                               SSD1306_CMD_SET_START_LINE | 0x00,
                                               SSD1306_CMD_CHARGE_PUMP,
                                               SSD1306_CHARGE_PUMP_ENABLE,
                                               SSD1306_CMD_MEMORY_MODE,
                                               SSD1306_MEMORY_MODE_HORIZONTAL,
                                               SSD1306_CMD_SEG_REMAP | SSD1306_SEG_REMAP_FLIP,
                                               SSD1306_CMD_COM_SCAN_DEC,
                                               SSD1306_CMD_SET_COM_PINS,
                                               SSD1306_COM_PINS_ALTERNATIVE,
                                               SSD1306_CMD_SET_CONTRAST,
                                               SSD1306_CONTRAST_DEFAULT,
                                               SSD1306_CMD_SET_PRECHARGE,
                                               SSD1306_PRECHARGE_DEFAULT,
                                               SSD1306_CMD_SET_VCOM_DETECT,
                                               SSD1306_VCOM_LEVEL_0_77,
                                               SSD1306_CMD_DISPLAY_ALL_ON_RESUME,
                                               SSD1306_CMD_NORMAL_DISPLAY};

SSD1306_StatusTypeDef SSD1306_Init(SSD1306_Handle_t *hssd, I2C_Bus_t *bus, uint8_t address) {
    SSD1306_StatusTypeDef status = SSD1306_OK;

    if (hssd == NULL || bus == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    log_info("SSD1306: initializing display at I2C address 0x%02X", address);

    memset(hssd, 0, sizeof(SSD1306_Handle_t));
    hssd->config.address = address;

    status = SSD1306_IO_Init(hssd, bus);
    if (status != SSD1306_OK) {
        return status;
    }

    status = SSD1306_IO_WriteCommands(hssd, SSD1306_InitSequence, sizeof(SSD1306_InitSequence));
    if (status != SSD1306_OK) {
        return status;
    }

    /* The buffer and refresh calls both refuse an uninitialised handle. */
    hssd->initialized = true;

    /* GDDRAM powers up with undefined content, so it has to be blanked before
       the panel is switched on, not just the RAM buffer. */
    (void)SSD1306_Clear(hssd);
    status = SSD1306_UpdateScreen(hssd);
    if (status != SSD1306_OK) {
        hssd->initialized = false;
        return status;
    }

    status = SSD1306_IO_WriteCommand(hssd, SSD1306_CMD_DISPLAY_ON);
    if (status != SSD1306_OK) {
        hssd->initialized = false;
        return status;
    }

    log_info("SSD1306: display initialized successfully");

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_DeInit(SSD1306_Handle_t *hssd) {
    SSD1306_StatusTypeDef status = SSD1306_OK;

    SSD1306_CHECK_HANDLE(hssd);

    status = SSD1306_IO_WriteCommand(hssd, SSD1306_CMD_DISPLAY_OFF);
    hssd->initialized = false;

    return status;
}

SSD1306_StatusTypeDef SSD1306_DisplayOn(SSD1306_Handle_t *hssd, bool enable) {
    SSD1306_CHECK_HANDLE(hssd);

    return SSD1306_IO_WriteCommand(hssd, enable ? SSD1306_CMD_DISPLAY_ON : SSD1306_CMD_DISPLAY_OFF);
}

SSD1306_StatusTypeDef SSD1306_SetContrast(SSD1306_Handle_t *hssd, uint8_t contrast) {
    const uint8_t frame[] = {SSD1306_CMD_SET_CONTRAST, contrast};

    SSD1306_CHECK_HANDLE(hssd);

    return SSD1306_IO_WriteCommands(hssd, frame, sizeof(frame));
}

SSD1306_StatusTypeDef SSD1306_InvertDisplay(SSD1306_Handle_t *hssd, bool invert) {
    SSD1306_CHECK_HANDLE(hssd);

    return SSD1306_IO_WriteCommand(hssd, invert ? SSD1306_CMD_INVERT_DISPLAY
                                                : SSD1306_CMD_NORMAL_DISPLAY);
}
