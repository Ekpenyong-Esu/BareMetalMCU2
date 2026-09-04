/**
 ******************************************************************************
 * @file    ssd1306_io.c
 * @brief   SSD1306 I2C transport - internal to the driver
 ******************************************************************************
 */

#include "ssd1306_io.h"
#include "ssd1306_cmd.h"
#include "i2c.h"
#include "log.h"
#include <string.h>

#define SSD1306_IO_CMD_MAX 32U
#define SSD1306_IO_READY_TRIALS 3U

/* The HAL takes the 8-bit form; the config carries the datasheet 7-bit value. */
static uint16_t SSD1306_IO_Address(const SSD1306_Config_t *config) {
    return (uint16_t)((uint16_t)config->address << 1);
}

SSD1306_StatusTypeDef SSD1306_IO_Init(SSD1306_Handle_t *hssd, I2C_Bus_t *bus) {
    I2C_ConfigTypeDef busConfig = I2C_ConfigDefault();

    if (hssd == NULL || bus == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    if (hssd->config.address != SSD1306_I2C_ADDR && hssd->config.address != SSD1306_I2C_ADDR_ALT) {
        log_error("SSD1306: unsupported I2C address 0x%02X", hssd->config.address);
        return SSD1306_INVALID_PARAM;
    }

    if (I2C_DeviceInit(&hssd->device, bus, SSD1306_IO_Address(&hssd->config), &busConfig) !=
        I2C_OK) {
        log_error("SSD1306: I2C device registration failed");
        return SSD1306_ERROR;
    }

    if (I2C_IsDeviceReady(&hssd->device, SSD1306_IO_READY_TRIALS, I2C_TIMEOUT_DEFAULT) != I2C_OK) {
        log_error("SSD1306: no response from display at 0x%02X", hssd->config.address);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_IO_WriteCommand(SSD1306_Handle_t *hssd, uint8_t command) {
    uint8_t frame[2] = {SSD1306_CTRL_COMMAND, command};

    if (I2C_Master_Transmit(&hssd->device, frame, sizeof(frame), I2C_TIMEOUT_DEFAULT) != I2C_OK) {
        log_error("SSD1306: failed to write command 0x%02X", command);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_IO_WriteCommands(SSD1306_Handle_t *hssd, const uint8_t *commands,
                                               uint16_t count) {
    uint8_t frame[SSD1306_IO_CMD_MAX + 1];

    if (commands == NULL || count == 0U || count > SSD1306_IO_CMD_MAX) {
        return SSD1306_INVALID_PARAM;
    }

    frame[0] = SSD1306_CTRL_COMMAND;
    memcpy(&frame[1], commands, count);

    if (I2C_Master_Transmit(&hssd->device, frame, (uint16_t)(count + 1U), I2C_TIMEOUT_DEFAULT) !=
        I2C_OK) {
        log_error("SSD1306: failed to write %u command bytes", (unsigned)count);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}

SSD1306_StatusTypeDef SSD1306_IO_WriteData(SSD1306_Handle_t *hssd, const uint8_t *data,
                                           uint16_t size) {
    if (data == NULL || size == 0U || size > SSD1306_BUFFER_SIZE) {
        return SSD1306_INVALID_PARAM;
    }

    /* Staged so the control byte and the payload go out as a single transfer. */
    hssd->txBuffer[0] = SSD1306_CTRL_DATA;
    memcpy(&hssd->txBuffer[1], data, size);

    if (I2C_Master_Transmit(&hssd->device, hssd->txBuffer, (uint16_t)(size + 1U),
                            I2C_TIMEOUT_DEFAULT) != I2C_OK) {
        log_error("SSD1306: failed to write %u bytes of data", (unsigned)size);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}
