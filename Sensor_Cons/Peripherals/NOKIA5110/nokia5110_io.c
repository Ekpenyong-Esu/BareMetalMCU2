/**
 ******************************************************************************
 * @file    nokia5110_io.c
 * @brief   SPI and control-line transport for the Nokia 5110 LCD
 ******************************************************************************
 */

#include "nokia5110_io.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"
#include "stm32f4xx_hal.h"

#define NOKIA5110_SPI_TIMEOUT 1000U /* SPI timeout in ms */
#define NOKIA5110_RESET_LOW_MS 10U
#define NOKIA5110_RESET_HIGH_MS 10U /* Settle before the first command */

static NOKIA5110_StatusTypeDef NOKIA5110_IO_PinInit(GPIO_TypeDef *port, uint16_t pin,
                                                    uint32_t speed) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO driver enables the port clock */
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = speed;

    return (GPIO_Driver_Pin_Init(port, &GPIO_InitStruct) == HAL_OK) ? NOKIA5110_OK
                                                                    : NOKIA5110_ERROR;
}

static NOKIA5110_StatusTypeDef NOKIA5110_IO_Transmit(NOKIA5110_Handle_t *hnok, const uint8_t *data,
                                                     uint16_t size, GPIO_PinState dcState) {
    const NOKIA5110_Config_t *config = &hnok->Config;
    SPI_StatusTypeDef status = SPI_OK;

    GPIO_Driver_WritePin(config->DcPort, config->DcPin, dcState);
    GPIO_Driver_WritePin(config->CePort, config->CePin, GPIO_PIN_RESET);

    status = SPI_Transmit(&hnok->Device, (uint8_t *)(uintptr_t)data, size, NOKIA5110_SPI_TIMEOUT);

    GPIO_Driver_WritePin(config->CePort, config->CePin, GPIO_PIN_SET);

    if (status == SPI_TIMEOUT) {
        return NOKIA5110_TIMEOUT;
    }

    return (status == SPI_OK) ? NOKIA5110_OK : NOKIA5110_ERROR;
}

NOKIA5110_StatusTypeDef NOKIA5110_IO_Init(NOKIA5110_Handle_t *hnok) {
    const NOKIA5110_Config_t *config = NULL;
    NOKIA5110_StatusTypeDef status = NOKIA5110_OK;

    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    config = &hnok->Config;
    if (config->Bus == NULL || config->CePort == NULL || config->DcPort == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    if (config->RstPort != NULL) {
        status = NOKIA5110_IO_PinInit(config->RstPort, config->RstPin, GPIO_SPEED_FREQ_LOW);
        if (status != NOKIA5110_OK) {
            log_error("NOKIA5110: reset pin init failed");
            return status;
        }
        GPIO_Driver_WritePin(config->RstPort, config->RstPin, GPIO_PIN_SET);
    }

    status = NOKIA5110_IO_PinInit(config->CePort, config->CePin, GPIO_SPEED_FREQ_HIGH);
    if (status != NOKIA5110_OK) {
        log_error("NOKIA5110: chip-enable pin init failed");
        return status;
    }

    status = NOKIA5110_IO_PinInit(config->DcPort, config->DcPin, GPIO_SPEED_FREQ_HIGH);
    if (status != NOKIA5110_OK) {
        log_error("NOKIA5110: data/command pin init failed");
        return status;
    }

    /* Idle deselected before the device is registered, so no stray edge is latched. */
    GPIO_Driver_WritePin(config->CePort, config->CePin, GPIO_PIN_SET);
    GPIO_Driver_WritePin(config->DcPort, config->DcPin, GPIO_PIN_RESET);

    const SPI_ConfigTypeDef spiConfig = SPI_ConfigDefault();
    if (SPI_DeviceInit(&hnok->Device, config->Bus, &spiConfig) != SPI_OK) {
        log_error("NOKIA5110: SPI device registration failed");
        return NOKIA5110_ERROR;
    }

    return NOKIA5110_OK;
}

void NOKIA5110_IO_DeInit(NOKIA5110_Handle_t *hnok) {
    const NOKIA5110_Config_t *config = &hnok->Config;

    if (config->RstPort != NULL) {
        (void)GPIO_Driver_Pin_DeInit(config->RstPort, config->RstPin);
    }
    (void)GPIO_Driver_Pin_DeInit(config->CePort, config->CePin);
    (void)GPIO_Driver_Pin_DeInit(config->DcPort, config->DcPin);
}

void NOKIA5110_IO_Reset(NOKIA5110_Handle_t *hnok) {
    const NOKIA5110_Config_t *config = &hnok->Config;

    /* Without a wired RST the board's own power-on reset has to do this. */
    if (config->RstPort == NULL) {
        return;
    }

    GPIO_Driver_WritePin(config->RstPort, config->RstPin, GPIO_PIN_RESET);
    HAL_Delay(NOKIA5110_RESET_LOW_MS);
    GPIO_Driver_WritePin(config->RstPort, config->RstPin, GPIO_PIN_SET);
    HAL_Delay(NOKIA5110_RESET_HIGH_MS);
}

NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteCommand(NOKIA5110_Handle_t *hnok, uint8_t cmd) {
    return NOKIA5110_IO_Transmit(hnok, &cmd, 1U, GPIO_PIN_RESET);
}

NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteData(NOKIA5110_Handle_t *hnok, const uint8_t *data,
                                               uint16_t size) {
    if (data == NULL || size == 0U) {
        return NOKIA5110_INVALID_PARAM;
    }

    return NOKIA5110_IO_Transmit(hnok, data, size, GPIO_PIN_SET);
}
