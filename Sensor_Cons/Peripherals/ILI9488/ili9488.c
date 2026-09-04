/**
 ******************************************************************************
 * @file    ili9488.c
 * @brief   ILI9488 TFT LCD driver - lifecycle, orientation, full-screen fill
 ******************************************************************************
 */

#include "ili9488_core.h"
#include "ili9488_io.h"
#include "ili9488_cmd.h"
#include "log.h"
#include <string.h>

/* Panel bring-up parameters, applied in order by ILI9488_Init */
static const uint8_t ILI9488_ParamsPowerControl1[] = {0x17, 0x15};
static const uint8_t ILI9488_ParamsPowerControl2[] = {0x41};
static const uint8_t ILI9488_ParamsVcomControl1[] = {0x00, 0x12, 0x80};
static const uint8_t ILI9488_ParamsPixelFormat[] = {ILI9488_PIXEL_FORMAT_18BIT};
static const uint8_t ILI9488_ParamsFrameRate[] = {0xA0}; /* 60 Hz */
static const uint8_t ILI9488_ParamsPositiveGamma[] = {
    0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98, 0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00};
static const uint8_t ILI9488_ParamsNegativeGamma[] = {
    0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75, 0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00};

/* Fields ordered widest first so the table packs without padding. */
typedef struct {
    const uint8_t *params;
    uint16_t delayMs; /**< Settling time the panel needs after the command. */
    uint8_t command;
    uint8_t paramCount;
} ILI9488_InitStep_t;

/* Settling times the datasheet asks for after reset, sleep-out and display-on */
#define ILI9488_RESET_SETTLE_MS 150
#define ILI9488_SLEEP_OUT_SETTLE_MS 150
#define ILI9488_DISPLAY_ON_SETTLE_MS 100

#define ILI9488_INIT_STEP(cmd, table, delay)                                                       \
    { .command = (cmd), .params = (table), .paramCount = sizeof(table), .delayMs = (delay) }
#define ILI9488_INIT_STEP_NO_PARAMS(cmd, delay)                                                    \
    { .command = (cmd), .params = NULL, .paramCount = 0, .delayMs = (delay) }

/* Memory Access Control is deliberately absent: ILI9488_SetOrientation owns it. */
static const ILI9488_InitStep_t ILI9488_InitSequence[] = {
    ILI9488_INIT_STEP_NO_PARAMS(ILI9488_CMD_SOFTWARE_RESET, ILI9488_RESET_SETTLE_MS),
    ILI9488_INIT_STEP_NO_PARAMS(ILI9488_CMD_SLEEP_OUT, ILI9488_SLEEP_OUT_SETTLE_MS),
    ILI9488_INIT_STEP(ILI9488_CMD_POWER_CONTROL1, ILI9488_ParamsPowerControl1, 0),
    ILI9488_INIT_STEP(ILI9488_CMD_POWER_CONTROL2, ILI9488_ParamsPowerControl2, 0),
    ILI9488_INIT_STEP(ILI9488_CMD_VCOM_CONTROL1, ILI9488_ParamsVcomControl1, 0),
    ILI9488_INIT_STEP(ILI9488_CMD_PIXEL_FORMAT_SET, ILI9488_ParamsPixelFormat, 0),
    ILI9488_INIT_STEP(ILI9488_CMD_FRAME_RATE_NORMAL, ILI9488_ParamsFrameRate, 0),
    ILI9488_INIT_STEP(ILI9488_CMD_POSITIVE_GAMMA, ILI9488_ParamsPositiveGamma, 0),
    ILI9488_INIT_STEP(ILI9488_CMD_NEGATIVE_GAMMA, ILI9488_ParamsNegativeGamma, 0),
    ILI9488_INIT_STEP_NO_PARAMS(ILI9488_CMD_DISP_INVERSION_OFF, 0),
    ILI9488_INIT_STEP_NO_PARAMS(ILI9488_CMD_DISPLAY_ON, ILI9488_DISPLAY_ON_SETTLE_MS),
};

/**
 * @brief  Walk the bring-up table, stopping at the first command the panel
 *         does not acknowledge.
 */
static ILI9488_StatusTypeDef ILI9488_RunInitSequence(ILI9488_Handle_t *hili) {
    for (uint32_t i = 0U; i < (sizeof(ILI9488_InitSequence) / sizeof(ILI9488_InitSequence[0]));
         i++) {
        const ILI9488_InitStep_t *step = &ILI9488_InitSequence[i];
        ILI9488_StatusTypeDef status = ILI9488_IO_WriteCommand(hili, step->command);

        if (status != ILI9488_OK) {
            return status;
        }

        if (step->paramCount > 0U) {
            status = ILI9488_IO_WriteData(hili, step->params, step->paramCount);
            if (status != ILI9488_OK) {
                return status;
            }
        }

        if (step->delayMs > 0U) {
            HAL_Delay(step->delayMs);
        }
    }

    return ILI9488_OK;
}

/**
 * @brief  Translate an orientation into its MADCTL byte and panel geometry.
 */
static ILI9488_StatusTypeDef ILI9488_DescribeOrientation(ILI9488_Orientation_t orientation,
                                                         uint8_t *madctl, uint16_t *width,
                                                         uint16_t *height) {
    switch (orientation) {
        case ILI9488_ORIENTATION_PORTRAIT:
            *madctl = ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR;
            *width = ILI9488_WIDTH;
            *height = ILI9488_HEIGHT;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE:
            *madctl = ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
            *width = ILI9488_HEIGHT;
            *height = ILI9488_WIDTH;
            break;
        case ILI9488_ORIENTATION_PORTRAIT_REV:
            *madctl = ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR;
            *width = ILI9488_WIDTH;
            *height = ILI9488_HEIGHT;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE_REV:
            *madctl =
                ILI9488_MADCTL_MY | ILI9488_MADCTL_MX | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
            *width = ILI9488_HEIGHT;
            *height = ILI9488_WIDTH;
            break;
        default:
            return ILI9488_INVALID_PARAM;
    }

    return ILI9488_OK;
}

ILI9488_StatusTypeDef ILI9488_Init(ILI9488_Handle_t *hili, SPI_Bus_t *bus, GPIO_TypeDef *cs_port,
                                   uint16_t cs_pin, GPIO_TypeDef *dc_port, uint16_t dc_pin,
                                   GPIO_TypeDef *rst_port, uint16_t rst_pin) {
    ILI9488_StatusTypeDef status = ILI9488_OK;

    if (hili == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    memset(hili, 0, sizeof(ILI9488_Handle_t));

    hili->config.bus = bus;
    hili->config.cs_port = cs_port;
    hili->config.cs_pin = cs_pin;
    hili->config.dc_port = dc_port;
    hili->config.dc_pin = dc_pin;
    hili->config.rst_port = rst_port;
    hili->config.rst_pin = rst_pin;
    hili->config.orientation = ILI9488_ORIENTATION_PORTRAIT;
    hili->width = ILI9488_WIDTH;
    hili->height = ILI9488_HEIGHT;

    status = ILI9488_IO_Init(hili);
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_RunInitSequence(hili);
    if (status != ILI9488_OK) {
        log_error("ILI9488: panel bring-up sequence failed");
        return status;
    }

    /* Mark ready so the orientation write, which guards on it, can proceed. */
    hili->initialized = true;

    status = ILI9488_SetOrientation(hili, ILI9488_ORIENTATION_PORTRAIT);
    if (status != ILI9488_OK) {
        hili->initialized = false;
        log_error("ILI9488: failed to apply default orientation");
    }

    return status;
}

ILI9488_StatusTypeDef ILI9488_SetOrientation(ILI9488_Handle_t *hili,
                                             ILI9488_Orientation_t orientation) {
    ILI9488_StatusTypeDef status = ILI9488_OK;
    uint8_t madctl = 0;
    uint16_t width = 0;
    uint16_t height = 0;

    ILI9488_CHECK_HANDLE(hili);

    status = ILI9488_DescribeOrientation(orientation, &madctl, &width, &height);
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_IO_WriteCommand(hili, ILI9488_CMD_MEMORY_ACCESS_CTL);
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_IO_WriteData(hili, &madctl, 1U);
    if (status != ILI9488_OK) {
        return status;
    }

    hili->config.orientation = orientation;
    hili->width = width;
    hili->height = height;

    /* The rotated panel may not contain the old cursor any more. */
    hili->currentX = 0;
    hili->currentY = 0;

    return ILI9488_OK;
}

ILI9488_StatusTypeDef ILI9488_Clear(ILI9488_Handle_t *hili, uint16_t color) {
    ILI9488_StatusTypeDef status = ILI9488_OK;

    ILI9488_CHECK_HANDLE(hili);

    status = ILI9488_IO_SetAddressWindow(hili, 0U, 0U, (uint16_t)(hili->width - 1U),
                                         (uint16_t)(hili->height - 1U));
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_IO_WriteCommand(hili, ILI9488_CMD_MEMORY_WRITE);
    if (status != ILI9488_OK) {
        return status;
    }

    return ILI9488_IO_WritePixels(hili, color, (uint32_t)hili->width * (uint32_t)hili->height);
}
