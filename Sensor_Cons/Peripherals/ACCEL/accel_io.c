/**
  ******************************************************************************
  * @file    accel_io.c
  * @brief   Register-level SPI access for the MMA8452Q
  ******************************************************************************
  */

#include "accel_io.h"
#include "accel_mma8452q.h"
#include "spi.h"
#include <string.h>

ACCEL_StatusTypeDef ACCEL_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t txBuffer[2];

    txBuffer[0] = ACCEL_SPI_WRITE_CMD | reg;
    txBuffer[1] = value;

    if (SPI_Transmit(txBuffer, sizeof(txBuffer), ACCEL_SPI_TIMEOUT) != SPI_OK)
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

    if (SPI_TransmitReceive(txBuffer, rxBuffer, sizeof(txBuffer), ACCEL_SPI_TIMEOUT) != SPI_OK)
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

    txBuffer[0] = ACCEL_SPI_READ_CMD | reg;

    if (SPI_TransmitReceive(txBuffer, rxBuffer, (uint16_t)(length + 1U), ACCEL_SPI_TIMEOUT) != SPI_OK)
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
