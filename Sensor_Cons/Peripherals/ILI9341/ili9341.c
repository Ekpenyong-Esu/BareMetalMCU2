/**
 * @file    ili9341.c
 * @brief   ILI9341 panel lifecycle and initialization sequence
 */

/* Includes ------------------------------------------------------------------*/
#include "ili9341_core.h"
#include "ili9341_io.h"
#include "log.h"

#include <string.h>

/* Private types -------------------------------------------------------------*/

/** @brief One controller command, optional parameters, and its required delay. */
typedef struct {
    const uint8_t *data; /**< Parameter bytes (NULL when none) */
    uint16_t delayMs;    /**< Wait after the command, 0 for none */
    uint8_t command;     /**< Controller command code */
    uint8_t dataCount;   /**< Number of parameter bytes */
} ILI9341_InitStep_t;

/** @brief One table row in command order, independent of the struct's packed field order. */
#define ILI9341_STEP(cmd, params, count, delay)                                                    \
    { .data = (params), .delayMs = (delay), .command = (cmd), .dataCount = (count) }

/* Private constants ---------------------------------------------------------*/

static const uint8_t s_powerOnSequence[] = {0xC3, 0x08, 0x50};
static const uint8_t s_powerB[] = {0x00, 0xC1, 0x30};
static const uint8_t s_powerSequence[] = {0x64, 0x03, 0x12, 0x81};
static const uint8_t s_dtca[] = {0x85, 0x00, 0x78};
static const uint8_t s_powerA[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
static const uint8_t s_dtcB[] = {0x00, 0x00};
static const uint8_t s_vcom1[] = {0x45, 0x15};
static const uint8_t s_frameRate[] = {0x00, 0x1B};
static const uint8_t s_displayFunction[] = {0x0A, 0xA2};
static const uint8_t s_displayFunctionRgb[] = {0x0A, 0xA7, 0x27, 0x04};
static const uint8_t s_columnAddress[] = {0x00, 0x00, 0x00, ILI9341_COL_END};
static const uint8_t s_pageAddress[] = {0x00, 0x00, 0x01, ILI9341_PAGE_END};
static const uint8_t s_interface[] = {0x01, 0x00, 0x06};
static const uint8_t s_positiveGamma[] = {0x0F, 0x29, 0x24, 0x0C, 0x0E, 0x09, 0x4E, 0x78,
                                          0x3C, 0x09, 0x13, 0x05, 0x17, 0x11, 0x00};
static const uint8_t s_negativeGamma[] = {0x00, 0x16, 0x1B, 0x04, 0x11, 0x07, 0x31, 0x33,
                                          0x42, 0x05, 0x0C, 0x0A, 0x28, 0x2F, 0x0F};

/** @brief Ordered ST panel setup sequence and its controller-specific parameters. */
static const ILI9341_InitStep_t s_initSequence[] = {
    ILI9341_STEP(ILI9341_SWRESET, NULL, 0U, ILI9341_INIT_DELAY_MS),
    ILI9341_STEP(ILI9341_POWER_ON_SEQUENCE, s_powerOnSequence, sizeof(s_powerOnSequence), 0U),
    ILI9341_STEP(ILI9341_POWERB, s_powerB, sizeof(s_powerB), 0U),
    ILI9341_STEP(ILI9341_POWER_SEQ, s_powerSequence, sizeof(s_powerSequence), 0U),
    ILI9341_STEP(ILI9341_DTCA, s_dtca, sizeof(s_dtca), 0U),
    ILI9341_STEP(ILI9341_POWERA, s_powerA, sizeof(s_powerA), 0U),
    ILI9341_STEP(ILI9341_PRC, (const uint8_t[]){0x20}, 1U, 0U),
    ILI9341_STEP(ILI9341_DTCB, s_dtcB, sizeof(s_dtcB), 0U),
    ILI9341_STEP(ILI9341_FRC, s_frameRate, sizeof(s_frameRate), 0U),
    ILI9341_STEP(ILI9341_DFC, s_displayFunction, sizeof(s_displayFunction), 0U),
    ILI9341_STEP(ILI9341_POWER1, (const uint8_t[]){0x10}, 1U, 0U),
    ILI9341_STEP(ILI9341_POWER2, (const uint8_t[]){0x10}, 1U, 0U),
    ILI9341_STEP(ILI9341_VCOM1, s_vcom1, sizeof(s_vcom1), 0U),
    ILI9341_STEP(ILI9341_VCOM2, (const uint8_t[]){0x90}, 1U, 0U),
    ILI9341_STEP(ILI9341_MAC, (const uint8_t[]){0xC8}, 1U, 0U),
    ILI9341_STEP(ILI9341_3GAMMA_EN, (const uint8_t[]){0x00}, 1U, 0U),
    ILI9341_STEP(ILI9341_RGB_INTERFACE, (const uint8_t[]){0xC2}, 1U, 0U),
    ILI9341_STEP(ILI9341_DFC, s_displayFunctionRgb, sizeof(s_displayFunctionRgb), 0U),
    ILI9341_STEP(ILI9341_COLUMN_ADDR, s_columnAddress, sizeof(s_columnAddress), 0U),
    ILI9341_STEP(ILI9341_PAGE_ADDR, s_pageAddress, sizeof(s_pageAddress), 0U),
    ILI9341_STEP(ILI9341_INTERFACE, s_interface, sizeof(s_interface), 0U),
    ILI9341_STEP(ILI9341_GRAM, NULL, 0U, ILI9341_INIT_DELAY_MS),
    ILI9341_STEP(ILI9341_GAMMA, (const uint8_t[]){0x01}, 1U, 0U),
    ILI9341_STEP(ILI9341_PGAMMA, s_positiveGamma, sizeof(s_positiveGamma), 0U),
    ILI9341_STEP(ILI9341_NGAMMA, s_negativeGamma, sizeof(s_negativeGamma), 0U),
    ILI9341_STEP(ILI9341_SLEEP_OUT, NULL, 0U, ILI9341_WAKE_DELAY_MS),
    ILI9341_STEP(ILI9341_DISPLAY_ON, NULL, 0U, 0U),
    ILI9341_STEP(ILI9341_GRAM, NULL, 0U, 0U),
};

/* Private functions ---------------------------------------------------------*/

static bool ILI9341_RunInitSequence(ILI9341_Handle_t *hili) {
    for (uint32_t index = 0; index < (sizeof(s_initSequence) / sizeof(s_initSequence[0]));
         index++) {
        const ILI9341_InitStep_t *step = &s_initSequence[index];

        if (ILI9341_WriteReg(hili, step->command) != SPI_OK) {
            return false;
        }

        for (uint8_t dataIndex = 0; dataIndex < step->dataCount; dataIndex++) {
            if (ILI9341_WriteData(hili, step->data[dataIndex]) != SPI_OK) {
                return false;
            }
        }

        if (step->delayMs > 0U) {
            HAL_Delay(step->delayMs);
        }
    }

    return true;
}

static bool ILI9341_IsReady(const ILI9341_Handle_t *hili) {
    return (hili != NULL) && hili->initialized;
}

/* Exported functions --------------------------------------------------------*/

uint16_t ILI9341_GetLcdPixelWidth(void) {
    return ILI9341_LCD_PIXEL_WIDTH;
}

uint16_t ILI9341_GetLcdPixelHeight(void) {
    return ILI9341_LCD_PIXEL_HEIGHT;
}

uint16_t ILI9341_ReadID(ILI9341_Handle_t *hili) {
    if (!ILI9341_IsReady(hili)) {
        return 0U;
    }

    /* The controller returns three bytes for this ID command. */
    uint32_t devId = ILI9341_ReadData(hili, ILI9341_READ_ID4, ILI9341_READ_ID4_SIZE);
    return (uint16_t)(devId & ILI9341_WORD_MASK);
}

bool ILI9341_DisplayOn(ILI9341_Handle_t *hili) {
    if (!ILI9341_IsReady(hili)) {
        return false;
    }

    log_debug("ILI9341: Turning display on");

    return ILI9341_WriteReg(hili, ILI9341_DISPLAY_ON) == SPI_OK;
}

bool ILI9341_DisplayOff(ILI9341_Handle_t *hili) {
    if (!ILI9341_IsReady(hili)) {
        return false;
    }

    log_debug("ILI9341: Turning display off");

    return ILI9341_WriteReg(hili, ILI9341_DISPLAY_OFF) == SPI_OK;
}

bool ILI9341_SleepIn(ILI9341_Handle_t *hili) {
    if (!ILI9341_IsReady(hili)) {
        return false;
    }

    log_debug("ILI9341: Entering sleep mode");

    if (ILI9341_WriteReg(hili, ILI9341_SLEEP_IN) != SPI_OK) {
        return false;
    }

    HAL_Delay(ILI9341_SLEEP_DELAY_MS);

    return true;
}

bool ILI9341_SleepOut(ILI9341_Handle_t *hili) {
    if (!ILI9341_IsReady(hili)) {
        return false;
    }

    log_debug("ILI9341: Exiting sleep mode");

    if (ILI9341_WriteReg(hili, ILI9341_SLEEP_OUT) != SPI_OK) {
        return false;
    }

    HAL_Delay(ILI9341_WAKE_DELAY_MS);

    return true;
}

bool ILI9341_Init(ILI9341_Handle_t *hili, const ILI9341_Config_t *config) {
    if (hili == NULL || config == NULL) {
        return false;
    }

    log_debug("ILI9341: Initializing display");

    memset(hili, 0, sizeof(*hili));
    hili->config = *config;

    if (ILI9341_IO_Init(hili) != SPI_OK) {
        log_error("ILI9341: transport init failed");
        return false;
    }

    if (!ILI9341_RunInitSequence(hili)) {
        log_error("ILI9341: panel init sequence failed");
        return false;
    }

    hili->initialized = true;

    log_debug("ILI9341: Initialization complete");

    return true;
}

void ILI9341_DeInit(ILI9341_Handle_t *hili) {
    if (!ILI9341_IsReady(hili)) {
        return;
    }

    ILI9341_IO_DeInit(hili);
    hili->initialized = false;
}
