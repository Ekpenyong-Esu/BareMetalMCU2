/**
  ******************************************************************************
  * @file    mems_io.c
  * @brief   Register access over SPI for the L3GD20
  ******************************************************************************
  */

#include "mems_io.h"
#include "mems_hw.h"
#include "mems_l3gd20.h"
#include "spi_transfer.h"

/**
 * @brief Frame one transfer: assert CS, send the command byte, move the payload.
 * @note  CS is released on every path so a failed transfer cannot leave the bus
 *        selected.
 */
static MEMS_StatusTypeDef MEMS_Transfer(MEMS_HandleTypeDef *hmems, uint8_t command,
                                       uint8_t *rxData, const uint8_t *txData,
                                       uint8_t length)
{
    SPI_StatusTypeDef status;

    MEMS_CS_Low(hmems);

    status = SPI_Transmit(&hmems->device, &command, 1, MEMS_SPI_TIMEOUT);
    if (status == SPI_OK) {
        if (rxData != NULL) {
            status = SPI_Receive(&hmems->device, rxData, length, MEMS_SPI_TIMEOUT);
        } else {
            status = SPI_Transmit(&hmems->device, (uint8_t *)txData, length, MEMS_SPI_TIMEOUT);
        }
    }

    MEMS_CS_High(hmems);

    return (status == SPI_OK) ? MEMS_OK : MEMS_COMMUNICATION_ERROR;
}

static uint8_t MEMS_CommandByte(uint8_t addr, uint8_t length, bool read)
{
    uint8_t command = addr;

    if (read) {
        command |= L3GD20_READ_CMD;
    }
    if (length > 1U) {
        command |= L3GD20_MULTIPLEBYTE_CMD;
    }

    return command;
}

MEMS_StatusTypeDef MEMS_ReadRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                      uint8_t *data, uint8_t length)
{
    uint8_t command = 0U;

    if (hmems == NULL || data == NULL || length == 0U) {
        return MEMS_INVALID_PARAM;
    }
    if (!SPI_DeviceIsReady(&hmems->device)) {
        return MEMS_NOT_INITIALIZED;
    }

    command = MEMS_CommandByte(start_addr, length, true);
    return MEMS_Transfer(hmems, command, data, NULL, length);
}

MEMS_StatusTypeDef MEMS_WriteRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                       const uint8_t *data, uint8_t length)
{
    uint8_t command = 0U;

    if (hmems == NULL || data == NULL || length == 0U) {
        return MEMS_INVALID_PARAM;
    }
    if (!SPI_DeviceIsReady(&hmems->device)) {
        return MEMS_NOT_INITIALIZED;
    }

    command = MEMS_CommandByte(start_addr, length, false);
    return MEMS_Transfer(hmems, command, NULL, data, length);
}

MEMS_StatusTypeDef MEMS_ReadRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t *data)
{
    return MEMS_ReadRegisters(hmems, addr, data, 1U);
}

MEMS_StatusTypeDef MEMS_WriteRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t data)
{
    return MEMS_WriteRegisters(hmems, addr, &data, 1U);
}

MEMS_StatusTypeDef MEMS_UpdateRegister(MEMS_HandleTypeDef *hmems, uint8_t addr,
                                       uint8_t mask, uint8_t value)
{
    MEMS_StatusTypeDef status;
    uint8_t reg = 0U;

    status = MEMS_ReadRegister(hmems, addr, &reg);
    if (status != MEMS_OK) {
        return status;
    }

    reg = (uint8_t)((reg & (uint8_t)~mask) | (value & mask));

    return MEMS_WriteRegister(hmems, addr, reg);
}
