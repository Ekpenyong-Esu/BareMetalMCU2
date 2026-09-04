/**
 ******************************************************************************
 * @file    accel_io.c
 * @brief   Register-level SPI access for the MMA8452Q
 ******************************************************************************
 */

#include "accel_io.h"
#include "accel_mma8452q.h"
#include "spi.h"
#include "gpio.h"
#include <string.h>

/* Chip select is active low and idles high. */
static void ACCEL_Select(const ACCEL_Handle_t *haccel) {
    HAL_GPIO_WritePin(haccel->csPort, haccel->csPin, GPIO_PIN_RESET);
}

static void ACCEL_Deselect(const ACCEL_Handle_t *haccel) {
    HAL_GPIO_WritePin(haccel->csPort, haccel->csPin, GPIO_PIN_SET);
}

/* A handle without a registered device cannot talk. */
static ACCEL_StatusTypeDef ACCEL_CheckDevice(const ACCEL_Handle_t *haccel) {
    if (haccel == NULL) {
        return ACCEL_INVALID_PARAM;
    }
    if (!SPI_DeviceIsReady(&haccel->device)) {
        return ACCEL_NOT_READY;
    }

    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_IO_Init(ACCEL_Handle_t *haccel, SPI_Bus_t *bus) {
    const SPI_ConfigTypeDef config = SPI_ConfigDefault();
    GPIO_InitTypeDef gpioInit = {0};

    if (haccel == NULL || bus == NULL || haccel->csPort == NULL || haccel->csPin == 0U) {
        return ACCEL_INVALID_PARAM;
    }

    gpioInit.Pin = haccel->csPin;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    if (GPIO_Driver_Pin_Init(haccel->csPort, &gpioInit) != HAL_OK) {
        return ACCEL_ERROR;
    }

    /* Idle high before the first transfer, or the part sees a partial frame. */
    ACCEL_Deselect(haccel);

    return (SPI_DeviceInit(&haccel->device, bus, &config) == SPI_OK) ? ACCEL_OK : ACCEL_ERROR;
}

ACCEL_StatusTypeDef ACCEL_WriteRegister(ACCEL_Handle_t *haccel, uint8_t reg, uint8_t value) {
    uint8_t txBuffer[2];
    SPI_StatusTypeDef status = SPI_OK;

    ACCEL_StatusTypeDef ready = ACCEL_CheckDevice(haccel);
    if (ready != ACCEL_OK) {
        return ready;
    }

    txBuffer[0] = ACCEL_SPI_WRITE_CMD | reg;
    txBuffer[1] = value;

    ACCEL_Select(haccel);
    status = SPI_Transmit(&haccel->device, txBuffer, sizeof(txBuffer), ACCEL_SPI_TIMEOUT);
    ACCEL_Deselect(haccel);

    if (status != SPI_OK) {
        return ACCEL_ERROR;
    }

    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_ReadRegister(ACCEL_Handle_t *haccel, uint8_t reg, uint8_t *value) {
    ACCEL_StatusTypeDef ready = ACCEL_CheckDevice(haccel);
    if (ready != ACCEL_OK) {
        return ready;
    }
    if (value == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t txBuffer[2] = {(uint8_t)(ACCEL_SPI_READ_CMD | reg), 0x00U};
    uint8_t rxBuffer[2];
    SPI_StatusTypeDef status = SPI_OK;

    ACCEL_Select(haccel);
    status = SPI_TransmitReceive(&haccel->device, txBuffer, rxBuffer, sizeof(txBuffer),
                                 ACCEL_SPI_TIMEOUT);
    ACCEL_Deselect(haccel);

    if (status != SPI_OK) {
        return ACCEL_ERROR;
    }

    *value = rxBuffer[1];
    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_ReadRegisters(ACCEL_Handle_t *haccel, uint8_t reg, uint8_t *buffer,
                                        uint16_t length) {
    ACCEL_StatusTypeDef ready = ACCEL_CheckDevice(haccel);
    if (ready != ACCEL_OK) {
        return ready;
    }
    if (buffer == NULL || length == 0U || length > ACCEL_BURST_MAX) {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t txBuffer[ACCEL_BURST_MAX + 1U] = {0};
    uint8_t rxBuffer[ACCEL_BURST_MAX + 1U];
    SPI_StatusTypeDef status = SPI_OK;

    txBuffer[0] = ACCEL_SPI_READ_CMD | reg;

    /* The whole burst shares one chip-select assertion; releasing it mid-read
       would restart the part's address counter. */
    ACCEL_Select(haccel);
    status = SPI_TransmitReceive(&haccel->device, txBuffer, rxBuffer, (uint16_t)(length + 1U),
                                 ACCEL_SPI_TIMEOUT);
    ACCEL_Deselect(haccel);

    if (status != SPI_OK) {
        return ACCEL_ERROR;
    }

    memcpy(buffer, &rxBuffer[1], length);
    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_UpdateRegister(ACCEL_Handle_t *haccel, uint8_t reg, uint8_t mask,
                                         uint8_t value) {
    uint8_t current = 0;

    ACCEL_StatusTypeDef status = ACCEL_ReadRegister(haccel, reg, &current);
    if (status != ACCEL_OK) {
        return status;
    }

    current = (uint8_t)((current & (uint8_t)~mask) | (value & mask));

    return ACCEL_WriteRegister(haccel, reg, current);
}
