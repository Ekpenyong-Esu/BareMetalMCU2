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

/* The MMA8452Q's slot on the shared bus; the settings are the board default. */
static SPI_Device_t s_device;

/* Chip select is active low and idles high. */
static void ACCEL_Select(void)
{
    HAL_GPIO_WritePin(ACCEL_CS_PORT, ACCEL_CS_PIN, GPIO_PIN_RESET);
}

static void ACCEL_Deselect(void)
{
    HAL_GPIO_WritePin(ACCEL_CS_PORT, ACCEL_CS_PIN, GPIO_PIN_SET);
}

ACCEL_StatusTypeDef ACCEL_IO_Init(void)
{
    const SPI_ConfigTypeDef config = SPI_ConfigDefault();
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = ACCEL_CS_PIN;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    (void)GPIO_Driver_Pin_Init(ACCEL_CS_PORT, &gpioInit);

    /* Idle high before the first transfer, or the part sees a partial frame. */
    ACCEL_Deselect();

    return (SPI_DeviceInit(&s_device, &config) == SPI_OK) ? ACCEL_OK : ACCEL_ERROR;
}

ACCEL_StatusTypeDef ACCEL_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t txBuffer[2];
    SPI_StatusTypeDef status;

    txBuffer[0] = ACCEL_SPI_WRITE_CMD | reg;
    txBuffer[1] = value;

    ACCEL_Select();
    status = SPI_Transmit(&s_device, txBuffer, sizeof(txBuffer), ACCEL_SPI_TIMEOUT);
    ACCEL_Deselect();

    if (status != SPI_OK)
    {
        return ACCEL_ERROR;
    }

    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_ReadRegister(uint8_t reg, uint8_t *value)
{
    if (value == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t txBuffer[2] = { (uint8_t)(ACCEL_SPI_READ_CMD | reg), 0x00U };
    uint8_t rxBuffer[2];
    SPI_StatusTypeDef status;

    ACCEL_Select();
    status = SPI_TransmitReceive(&s_device, txBuffer, rxBuffer, sizeof(txBuffer),
                                 ACCEL_SPI_TIMEOUT);
    ACCEL_Deselect();

    if (status != SPI_OK)
    {
        return ACCEL_ERROR;
    }

    *value = rxBuffer[1];
    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_ReadRegisters(uint8_t reg, uint8_t *buffer, uint16_t length)
{
    if (buffer == NULL || length == 0U || length > ACCEL_BURST_MAX)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t txBuffer[ACCEL_BURST_MAX + 1U] = {0};
    uint8_t rxBuffer[ACCEL_BURST_MAX + 1U];
    SPI_StatusTypeDef status;

    txBuffer[0] = ACCEL_SPI_READ_CMD | reg;

    /* The whole burst shares one chip-select assertion; releasing it mid-read
       would restart the part's address counter. */
    ACCEL_Select();
    status = SPI_TransmitReceive(&s_device, txBuffer, rxBuffer,
                                 (uint16_t)(length + 1U), ACCEL_SPI_TIMEOUT);
    ACCEL_Deselect();

    if (status != SPI_OK)
    {
        return ACCEL_ERROR;
    }

    memcpy(buffer, &rxBuffer[1], length);
    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_UpdateRegister(uint8_t reg, uint8_t mask, uint8_t value)
{
    uint8_t current = 0;

    ACCEL_StatusTypeDef status = ACCEL_ReadRegister(reg, &current);
    if (status != ACCEL_OK)
    {
        return status;
    }

    current = (uint8_t)((current & (uint8_t)~mask) | (value & mask));

    return ACCEL_WriteRegister(reg, current);
}
