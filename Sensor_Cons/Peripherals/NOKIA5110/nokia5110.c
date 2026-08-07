/**
  ******************************************************************************
  * @file    nokia5110.c
  * @brief   Lifecycle, configuration and framebuffer transfer
  ******************************************************************************
  */

#include "nokia5110_core.h"
#include "nokia5110_io.h"
#include "log.h"
#include <string.h>

#define NOKIA5110_EXTENDED_SET   (NOKIA5110_CMD_FUNCTION_SET | NOKIA5110_FUNCTION_H)
#define NOKIA5110_STANDARD_SET   (NOKIA5110_CMD_FUNCTION_SET)
#define NOKIA5110_POWER_DOWN     (NOKIA5110_CMD_FUNCTION_SET | NOKIA5110_FUNCTION_PD)

static uint8_t NOKIA5110_ModeCommand(NOKIA5110_DisplayMode_t mode)
{
    uint8_t bits;

    switch (mode) {
        case NOKIA5110_MODE_BLANK:   bits = NOKIA5110_DISPLAY_BLANK;   break;
        case NOKIA5110_MODE_ALL_ON:  bits = NOKIA5110_DISPLAY_ALL_ON;  break;
        case NOKIA5110_MODE_INVERSE: bits = NOKIA5110_DISPLAY_INVERSE; break;
        case NOKIA5110_MODE_NORMAL:
        default:                     bits = NOKIA5110_DISPLAY_NORMAL;  break;
    }

    return (uint8_t)(NOKIA5110_CMD_DISPLAY_CONTROL | bits);
}

static NOKIA5110_StatusTypeDef NOKIA5110_ValidateConfig(const NOKIA5110_Config_t *config)
{
    if (config->Contrast > NOKIA5110_CONTRAST_MAX ||
        config->TemperatureCoeff > NOKIA5110_TEMP_COEFF_MAX ||
        config->BiasSystem > NOKIA5110_BIAS_MAX ||
        config->Mode > NOKIA5110_MODE_INVERSE) {
        return NOKIA5110_INVALID_PARAM;
    }

    return NOKIA5110_OK;
}

/** Send a command sequence, stopping at the first failure. */
static NOKIA5110_StatusTypeDef NOKIA5110_WriteCommands(const uint8_t *commands, uint8_t count)
{
    for (uint8_t i = 0U; i < count; i++) {
        NOKIA5110_StatusTypeDef status = NOKIA5110_IO_WriteCommand(commands[i]);

        if (status != NOKIA5110_OK) {
            return status;
        }
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_Init(NOKIA5110_Handle_t *hnok)
{
    NOKIA5110_Config_t defaultConfig;
    NOKIA5110_StatusTypeDef status;

    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    memset(hnok, 0, sizeof(*hnok));

    status = NOKIA5110_IO_Init();
    if (status != NOKIA5110_OK) {
        return status;
    }

    NOKIA5110_IO_Reset();

    /* Marked ready so the configuration and clear below pass their guards;
       cleared again if either fails. */
    hnok->IsInitialized = true;

    defaultConfig = NOKIA5110_GetDefaultConfig();

    status = NOKIA5110_Config(hnok, &defaultConfig);
    if (status == NOKIA5110_OK) {
        status = NOKIA5110_Clear(hnok);
    }

    if (status != NOKIA5110_OK) {
        hnok->IsInitialized = false;
        log_error("NOKIA5110: display initialization failed");
        return status;
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_DeInit(NOKIA5110_Handle_t *hnok)
{
    NOKIA5110_CHECK_HANDLE(hnok);

    (void)NOKIA5110_Clear(hnok);
    (void)NOKIA5110_IO_WriteCommand(NOKIA5110_POWER_DOWN);

    NOKIA5110_IO_DeInit();

    hnok->IsInitialized = false;

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_Config(NOKIA5110_Handle_t *hnok, const NOKIA5110_Config_t *config)
{
    NOKIA5110_StatusTypeDef status;
    uint8_t commands[6];

    NOKIA5110_CHECK_HANDLE(hnok);

    if (config == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    status = NOKIA5110_ValidateConfig(config);
    if (status != NOKIA5110_OK) {
        return status;
    }

    commands[0] = NOKIA5110_EXTENDED_SET;
    commands[1] = (uint8_t)(NOKIA5110_CMD_VOP | config->Contrast);
    commands[2] = (uint8_t)(NOKIA5110_CMD_TEMP_CONTROL | config->TemperatureCoeff);
    commands[3] = (uint8_t)(NOKIA5110_CMD_BIAS_SYSTEM | config->BiasSystem);
    commands[4] = NOKIA5110_STANDARD_SET;
    commands[5] = NOKIA5110_ModeCommand(config->Mode);

    status = NOKIA5110_WriteCommands(commands, (uint8_t)(sizeof(commands) / sizeof(commands[0])));
    if (status != NOKIA5110_OK) {
        return status;
    }

    /* Only adopt the settings the controller actually accepted. */
    hnok->Config = *config;

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_Clear(NOKIA5110_Handle_t *hnok)
{
    NOKIA5110_CHECK_HANDLE(hnok);

    memset(hnok->Buffer, 0, sizeof(hnok->Buffer));

    return NOKIA5110_Update(hnok);
}

NOKIA5110_StatusTypeDef NOKIA5110_Update(NOKIA5110_Handle_t *hnok)
{
    NOKIA5110_StatusTypeDef status;
    const uint8_t address[] = { NOKIA5110_CMD_SET_Y_ADDR, NOKIA5110_CMD_SET_X_ADDR };

    NOKIA5110_CHECK_HANDLE(hnok);

    status = NOKIA5110_WriteCommands(address, (uint8_t)(sizeof(address) / sizeof(address[0])));
    if (status != NOKIA5110_OK) {
        return status;
    }

    for (uint8_t row = 0U; row < NOKIA5110_ROWS; row++) {
        status = NOKIA5110_IO_WriteData(hnok->Buffer[row], NOKIA5110_WIDTH);
        if (status != NOKIA5110_OK) {
            return status;
        }
    }

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_SetContrast(NOKIA5110_Handle_t *hnok, uint8_t contrast)
{
    NOKIA5110_StatusTypeDef status;
    uint8_t commands[3];

    NOKIA5110_CHECK_HANDLE(hnok);

    if (contrast > NOKIA5110_CONTRAST_MAX) {
        return NOKIA5110_INVALID_PARAM;
    }

    commands[0] = NOKIA5110_EXTENDED_SET;
    commands[1] = (uint8_t)(NOKIA5110_CMD_VOP | contrast);
    commands[2] = NOKIA5110_STANDARD_SET;

    status = NOKIA5110_WriteCommands(commands, (uint8_t)(sizeof(commands) / sizeof(commands[0])));
    if (status != NOKIA5110_OK) {
        return status;
    }

    hnok->Config.Contrast = contrast;

    return NOKIA5110_OK;
}

NOKIA5110_StatusTypeDef NOKIA5110_SetMode(NOKIA5110_Handle_t *hnok, NOKIA5110_DisplayMode_t mode)
{
    NOKIA5110_StatusTypeDef status;

    NOKIA5110_CHECK_HANDLE(hnok);

    if (mode > NOKIA5110_MODE_INVERSE) {
        return NOKIA5110_INVALID_PARAM;
    }

    status = NOKIA5110_IO_WriteCommand(NOKIA5110_ModeCommand(mode));
    if (status != NOKIA5110_OK) {
        return status;
    }

    hnok->Config.Mode = mode;

    return NOKIA5110_OK;
}

uint8_t NOKIA5110_GetWidth(void)
{
    return NOKIA5110_WIDTH;
}

uint8_t NOKIA5110_GetHeight(void)
{
    return NOKIA5110_HEIGHT;
}

NOKIA5110_Config_t NOKIA5110_GetDefaultConfig(void)
{
    NOKIA5110_Config_t config = {
        .Contrast = 0x40,
        .TemperatureCoeff = NOKIA5110_TEMP_COEFF_0,
        /* 1:48 is the bias the datasheet pairs with this panel's 48-row
           multiplex rate; 1:40 leaves the contrast range unusable. */
        .BiasSystem = NOKIA5110_BIAS_1_48,
        .Mode = NOKIA5110_MODE_NORMAL
    };

    return config;
}
